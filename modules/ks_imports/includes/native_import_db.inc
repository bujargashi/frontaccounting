<?php

function ks_import_user()
{
    return isset($_SESSION['wa_current_user']->username)
        ? $_SESSION['wa_current_user']->username : '';
}

function ks_import_sql_date($date)
{
    return db_escape(date2sql($date));
}

function ks_import_num($name)
{
    return input_num($name, 0);
}

function ks_import_post_data()
{
    return array(
        'dud_no' => trim(get_post('ks_dud_no')),
        'dud_date' => get_post('ks_dud_date'),
        'clearance_date' => get_post('ks_clearance_date'),
        'customs_office' => trim(get_post('ks_customs_office')),
        'origin_country' => trim(get_post('ks_origin_country')),
        'incoterm' => trim(get_post('ks_incoterm')),
        'customs_exchange_rate' => ks_import_num('ks_customs_exchange_rate'),
        'tariff_codes' => trim(get_post('ks_tariff_codes')),
        'customs_value' => ks_import_num('ks_customs_value'),
        'customs_duty' => ks_import_num('ks_customs_duty'),
        'excise' => ks_import_num('ks_excise'),
        'import_vat_base' => ks_import_num('ks_import_vat_base'),
        'import_vat' => ks_import_num('ks_import_vat'),
        'eur1_reference' => trim(get_post('ks_eur1_reference')),
        'import_notes' => trim(get_post('ks_import_notes'))
    );
}

function ks_import_validate_data($data)
{
    $ok = true;
    if ($data['dud_no'] == '') {
        display_error(_('Numri i DUD-it eshte i detyrueshem.'));
        set_focus('ks_dud_no');
        $ok = false;
    }
    if (!is_date($data['dud_date'])) {
        display_error(_('Data e DUD-it nuk eshte valide.'));
        set_focus('ks_dud_date');
        $ok = false;
    }
    if (!is_date($data['clearance_date'])) {
        display_error(_('Data e zhdoganimit nuk eshte valide.'));
        set_focus('ks_clearance_date');
        $ok = false;
    }
    if ($data['origin_country'] == '') {
        display_error(_('Shteti i origjines eshte i detyrueshem.'));
        set_focus('ks_origin_country');
        $ok = false;
    }
    if ($data['customs_exchange_rate'] <= 0) {
        display_error(_('Kursi doganor duhet te jete me i madh se zero.'));
        set_focus('ks_customs_exchange_rate');
        $ok = false;
    }
    if ($data['customs_value'] <= 0) {
        display_error(_('Vlera doganore duhet te jete me e madhe se zero.'));
        set_focus('ks_customs_value');
        $ok = false;
    }
    foreach (array('customs_duty', 'excise', 'import_vat_base', 'import_vat') as $field) {
        if ($data[$field] < 0) {
            display_error(_('Shumat doganore dhe tatimore nuk mund te jene negative.'));
            $ok = false;
            break;
        }
    }
    return $ok;
}

function ks_import_save_document($trans_no, $supplier_id, $data)
{
    $now = db_escape(date('Y-m-d H:i:s'));
    $user = db_escape(ks_import_user());
    $sql = "INSERT INTO ".TB_PREF."ks_import_documents
        (trans_type, trans_no, supplier_id, dud_no, dud_date, clearance_date,
         customs_office, origin_country, incoterm, customs_exchange_rate,
         tariff_codes, customs_value, customs_duty, excise, import_vat_base,
         import_vat, eur1_reference, import_notes, created_by, created_at,
         updated_by, updated_at)
        VALUES (".ST_SUPPINVOICE.",".(int)$trans_no.",".(int)$supplier_id.",".
        db_escape($data['dud_no']).",".ks_import_sql_date($data['dud_date']).",".
        ks_import_sql_date($data['clearance_date']).",".
        db_escape($data['customs_office']).",".db_escape($data['origin_country']).",".
        db_escape($data['incoterm']).",".(float)$data['customs_exchange_rate'].",".
        db_escape($data['tariff_codes']).",".(float)$data['customs_value'].",".
        (float)$data['customs_duty'].",".(float)$data['excise'].",".
        (float)$data['import_vat_base'].",".(float)$data['import_vat'].",".
        db_escape($data['eur1_reference']).",".db_escape($data['import_notes']).",".
        $user.",".$now.",".$user.",".$now.")
        ON DUPLICATE KEY UPDATE supplier_id=VALUES(supplier_id),
        dud_no=VALUES(dud_no), dud_date=VALUES(dud_date),
        clearance_date=VALUES(clearance_date), customs_office=VALUES(customs_office),
        origin_country=VALUES(origin_country), incoterm=VALUES(incoterm),
        customs_exchange_rate=VALUES(customs_exchange_rate),
        tariff_codes=VALUES(tariff_codes), customs_value=VALUES(customs_value),
        customs_duty=VALUES(customs_duty), excise=VALUES(excise),
        import_vat_base=VALUES(import_vat_base), import_vat=VALUES(import_vat),
        eur1_reference=VALUES(eur1_reference), import_notes=VALUES(import_notes),
        updated_by=VALUES(updated_by), updated_at=VALUES(updated_at)";
    db_query($sql, _('Te dhenat e importit nuk mund te ruhen.'));
}

function ks_import_get_document($trans_no)
{
    $sql = "SELECT i.*, s.supp_name, t.reference, t.supp_reference,
        t.tran_date, t.ov_amount, t.ov_gst, t.alloc
        FROM ".TB_PREF."ks_import_documents i
        LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=i.supplier_id
        LEFT JOIN ".TB_PREF."supp_trans t ON t.type=i.trans_type
            AND t.trans_no=i.trans_no
        WHERE i.trans_type=".ST_SUPPINVOICE." AND i.trans_no=".(int)$trans_no;
    $res = db_query($sql, _('Blerja nga importi nuk mund te lexohet.'));
    return db_fetch_assoc($res);
}

function ks_import_query($from, $to, $supplier_id = 0, $search = '')
{
    $sql = "SELECT i.*, s.supp_name, t.reference, t.supp_reference,
        t.tran_date, t.ov_amount, t.ov_gst, t.alloc
        FROM ".TB_PREF."ks_import_documents i
        LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=i.supplier_id
        LEFT JOIN ".TB_PREF."supp_trans t ON t.type=i.trans_type
            AND t.trans_no=i.trans_no
        WHERE i.trans_type=".ST_SUPPINVOICE.
        " AND i.clearance_date>=".db_escape(date2sql($from)).
        " AND i.clearance_date<=".db_escape(date2sql($to));
    if ($supplier_id) {
        $sql .= " AND i.supplier_id=".(int)$supplier_id;
    }
    if (trim($search) != '') {
        $term = '%'.trim($search).'%';
        $sql .= " AND (i.dud_no LIKE ".db_escape($term).
            " OR t.supp_reference LIKE ".db_escape($term).
            " OR s.supp_name LIKE ".db_escape($term).")";
    }
    $sql .= " ORDER BY i.clearance_date, i.id";
    return db_query($sql, _('Regjistri i importeve nuk mund te lexohet.'));
}
