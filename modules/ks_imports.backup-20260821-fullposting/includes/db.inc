<?php

function ks_import_user_name()
{
    return isset($_SESSION['wa_current_user']->username)
        ? $_SESSION['wa_current_user']->username : '';
}

function ks_import_sql_date($date)
{
    if (!$date) {
        return 'NULL';
    }
    return db_escape(date2sql($date));
}

function ks_import_get($id)
{
    $sql = "SELECT i.*, s.supp_name
        FROM ".TB_PREF."ks_imports i
        LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=i.supplier_id
        WHERE i.id=".(int)$id;
    $result = db_query($sql, _('Nuk mund te lexohet importi.'));
    return db_fetch_assoc($result);
}

function ks_import_is_draft($id)
{
    $row = ks_import_get($id);
    return $row && $row['status'] == 'draft';
}

function ks_import_find_by_dud($dud_no, $except_id = 0)
{
    if (trim($dud_no) === '') {
        return false;
    }
    $sql = "SELECT id FROM ".TB_PREF."ks_imports
        WHERE dud_no=".db_escape(trim($dud_no));
    if ($except_id) {
        $sql .= " AND id<>".(int)$except_id;
    }
    $result = db_query($sql, _('Nuk mund te kontrollohet numri DUD.'));
    return db_fetch($result);
}

function ks_import_next_reference()
{
    $year = date('Y');
    $prefix = 'IMP-'.$year.'-';
    $sql = "SELECT MAX(CAST(SUBSTRING(reference, ".(strlen($prefix) + 1).") AS UNSIGNED))
        FROM ".TB_PREF."ks_imports WHERE reference LIKE ".db_escape($prefix.'%');
    $result = db_query($sql, _('Nuk mund te gjenerohet referenca.'));
    $row = db_fetch($result);
    return $prefix.str_pad(((int)$row[0]) + 1, 4, '0', STR_PAD_LEFT);
}

function ks_import_save($id, $data)
{
    $now = date('Y-m-d H:i:s');
    $user = ks_import_user_name();
    $fields = array(
        'reference' => db_escape($data['reference']),
        'supplier_id' => (int)$data['supplier_id'],
        'supplier_invoice_no' => db_escape($data['supplier_invoice_no']),
        'supplier_invoice_date' => ks_import_sql_date($data['supplier_invoice_date']),
        'supplier_trans_no' => (int)$data['supplier_trans_no'],
        'origin_country' => db_escape($data['origin_country']),
        'dispatch_country' => db_escape($data['dispatch_country']),
        'dud_no' => db_escape($data['dud_no']),
        'dud_date' => ks_import_sql_date($data['dud_date']),
        'customs_office' => db_escape($data['customs_office']),
        'incoterm' => db_escape($data['incoterm']),
        'currency' => db_escape($data['currency']),
        'exchange_rate' => (float)$data['exchange_rate'],
        'goods_amount_currency' => (float)$data['goods_amount_currency'],
        'goods_amount_base' => (float)$data['goods_amount_base'],
        'transport_mode' => db_escape($data['transport_mode']),
        'transport_invoice_no' => db_escape($data['transport_invoice_no']),
        'transport_supplier_id' => (int)$data['transport_supplier_id'],
        'transport_trans_no' => (int)$data['transport_trans_no'],
        'transport_amount_base' => (float)$data['transport_amount_base'],
        'insurance_amount_base' => (float)$data['insurance_amount_base'],
        'handling_amount_base' => (float)$data['handling_amount_base'],
        'customs_value_override' => (float)$data['customs_value_override'],
        'customs_value_base' => (float)$data['customs_value_base'],
        'customs_duty_base' => (float)$data['customs_duty_base'],
        'excise_base' => (float)$data['excise_base'],
        'terminal_amount_base' => (float)$data['terminal_amount_base'],
        'forwarding_amount_base' => (float)$data['forwarding_amount_base'],
        'other_amount_base' => (float)$data['other_amount_base'],
        'import_vat_rate' => (float)$data['import_vat_rate'],
        'import_vat_base_override' => (float)$data['import_vat_base_override'],
        'import_vat_amount_override' => (float)$data['import_vat_amount_override'],
        'import_vat_base' => (float)$data['import_vat_base'],
        'import_vat_amount' => (float)$data['import_vat_amount'],
        'allocation_method' => db_escape($data['allocation_method']),
        'notes' => db_escape($data['notes']),
        'updated_by' => db_escape($user),
        'updated_at' => db_escape($now)
    );

    if ($id) {
        $parts = array();
        foreach ($fields as $field => $value) {
            $parts[] = $field.'='.$value;
        }
        $sql = "UPDATE ".TB_PREF."ks_imports SET ".implode(', ', $parts).
            " WHERE id=".(int)$id." AND status='draft'";
        db_query($sql, _('Importi nuk mund te ruhet.'));
        return (int)$id;
    }

    $fields['created_by'] = db_escape($user);
    $fields['created_at'] = db_escape($now);
    $sql = "INSERT INTO ".TB_PREF."ks_imports (".implode(', ', array_keys($fields)).")
        VALUES (".implode(', ', array_values($fields)).")";
    db_query($sql, _('Importi nuk mund te krijohet.'));
    return db_insert_id();
}

function ks_import_supplier_invoices($supplier_id, $limit = 200)
{
    $options = array(0 => _('Zgjidhni faturen e FrontAccounting'));
    if (!$supplier_id) {
        return $options;
    }
    $sql = "SELECT trans_no, supp_reference, reference, tran_date, ov_amount, ov_gst, rate
        FROM ".TB_PREF."supp_trans
        WHERE type=".ST_SUPPINVOICE." AND supplier_id=".(int)$supplier_id."
        ORDER BY tran_date DESC, trans_no DESC LIMIT ".(int)$limit;
    $result = db_query($sql, _('Faturat e furnitorit nuk mund te lexohen.'));
    while ($row = db_fetch_assoc($result)) {
        $label = sql2date($row['tran_date']).' | '.$row['supp_reference'].' | '.
            price_format($row['ov_amount'] + $row['ov_gst']).' | FA #'.$row['trans_no'];
        $options[$row['trans_no']] = $label;
    }
    return $options;
}

function ks_import_get_supplier_invoice($supplier_id, $trans_no)
{
    $sql = "SELECT t.*, s.curr_code, s.supp_name
        FROM ".TB_PREF."supp_trans t
        INNER JOIN ".TB_PREF."suppliers s ON s.supplier_id=t.supplier_id
        WHERE t.type=".ST_SUPPINVOICE." AND t.supplier_id=".(int)$supplier_id.
        " AND t.trans_no=".(int)$trans_no;
    $result = db_query($sql, _('Fatura e furnitorit nuk mund te lexohet.'));
    return db_fetch_assoc($result);
}

function ks_import_get_supplier_invoice_items($trans_no)
{
    $rows = array();
    $sql = "SELECT stock_id, description, quantity, unit_price
        FROM ".TB_PREF."supp_invoice_items
        WHERE supp_trans_type=".ST_SUPPINVOICE." AND supp_trans_no=".(int)$trans_no.
        " ORDER BY id";
    $result = db_query($sql, _('Rreshtat e fatures nuk mund te lexohen.'));
    while ($row = db_fetch_assoc($result)) {
        $rows[] = $row;
    }
    return $rows;
}

function ks_import_invoice_is_linked($supplier_id, $trans_no, $except_id = 0)
{
    $sql = "SELECT id FROM ".TB_PREF."ks_imports
        WHERE supplier_id=".(int)$supplier_id." AND supplier_trans_no=".(int)$trans_no;
    if ($except_id) {
        $sql .= " AND id<>".(int)$except_id;
    }
    $result = db_query($sql, _('Lidhja me faturen nuk mund te kontrollohet.'));
    return db_fetch($result);
}

function ks_import_delete($id)
{
    $row = ks_import_get($id);
    if (!$row || $row['status'] != 'draft') {
        return false;
    }
    db_query("DELETE FROM ".TB_PREF."ks_imports WHERE id=".(int)$id." AND status='draft'",
        _('Importi nuk mund te fshihet.'));
    return true;
}

function ks_import_items($import_id)
{
    $rows = array();
    $result = db_query("SELECT * FROM ".TB_PREF."ks_import_items
        WHERE import_id=".(int)$import_id." ORDER BY id",
        _('Artikujt nuk mund te lexohen.'));
    while ($row = db_fetch_assoc($result)) {
        $rows[] = $row;
    }
    return $rows;
}

function ks_import_add_item($import_id, $item)
{
    if (!ks_import_is_draft($import_id)) {
        return false;
    }
    $sql = "INSERT INTO ".TB_PREF."ks_import_items
        (import_id, stock_id, description, quantity, weight, invoice_value_base, manual_share)
        VALUES (".(int)$import_id.", ".db_escape($item['stock_id']).", ".
        db_escape($item['description']).", ".(float)$item['quantity'].", ".
        (float)$item['weight'].", ".(float)$item['invoice_value_base'].", ".
        (float)$item['manual_share'].")";
    db_query($sql, _('Artikulli nuk mund te shtohet.'));
    return true;
}

function ks_import_stock_description($stock_id)
{
    if (!$stock_id) {
        return '';
    }
    $sql = "SELECT description FROM ".TB_PREF."stock_master
        WHERE stock_id=".db_escape($stock_id)." AND inactive=0";
    $result = db_query($sql, _('Artikulli nuk mund te lexohet.'));
    $row = db_fetch($result);
    return $row ? $row[0] : '';
}

function ks_import_delete_item($import_id, $item_id)
{
    $sql = "DELETE d FROM ".TB_PREF."ks_import_items d
        INNER JOIN ".TB_PREF."ks_imports i ON i.id=d.import_id
        WHERE d.id=".(int)$item_id." AND d.import_id=".(int)$import_id." AND i.status='draft'";
    db_query($sql, _('Artikulli nuk mund te fshihet.'));
}

function ks_import_store_allocations($import_id, $items)
{
    foreach ($items as $item) {
        $sql = "UPDATE ".TB_PREF."ks_import_items SET
            duty_allocated=".(float)$item['duty_allocated'].",
            excise_allocated=".(float)$item['excise_allocated'].",
            transport_allocated=".(float)$item['transport_allocated'].",
            insurance_allocated=".(float)$item['insurance_allocated'].",
            handling_allocated=".(float)$item['handling_allocated'].",
            terminal_allocated=".(float)$item['terminal_allocated'].",
            forwarding_allocated=".(float)$item['forwarding_allocated'].",
            other_allocated=".(float)$item['other_allocated'].",
            landed_cost=".(float)$item['landed_cost'].",
            unit_landed_cost=".(float)$item['unit_landed_cost']."
            WHERE id=".(int)$item['id']." AND import_id=".(int)$import_id;
        db_query($sql, _('Shperndarja e kostos nuk mund te ruhet.'));
    }
}

function ks_import_setting($key, $default = '')
{
    $sql = "SELECT setting_value FROM ".TB_PREF."ks_import_settings
        WHERE setting_key=".db_escape($key);
    $result = db_query($sql, _('Cilesimi nuk mund te lexohet.'));
    $row = db_fetch($result);
    return $row ? $row[0] : $default;
}

function ks_import_set_setting($key, $value)
{
    $sql = "INSERT INTO ".TB_PREF."ks_import_settings (setting_key, setting_value)
        VALUES (".db_escape($key).", ".db_escape($value).")
        ON DUPLICATE KEY UPDATE setting_value=VALUES(setting_value)";
    db_query($sql, _('Cilesimi nuk mund te ruhet.'));
}

function ks_import_mark_ready($id)
{
    $sql = "UPDATE ".TB_PREF."ks_imports SET status='ready', updated_by=".
        db_escape(ks_import_user_name()).", updated_at=".db_escape(date('Y-m-d H:i:s')).
        " WHERE id=".(int)$id." AND status='draft'";
    db_query($sql, _('Statusi nuk mund te ndryshohet.'));
}

function ks_import_reopen($id)
{
    $sql = "UPDATE ".TB_PREF."ks_imports SET status='draft', updated_by=".
        db_escape(ks_import_user_name()).", updated_at=".db_escape(date('Y-m-d H:i:s')).
        " WHERE id=".(int)$id." AND status='ready'";
    db_query($sql, _('Statusi nuk mund te ndryshohet.'));
}
