<?php

if (!defined('KS_IMPORT_PAYMENT_SPLIT_ENABLED')) {
    define('KS_IMPORT_PAYMENT_SPLIT_ENABLED', true);
}

function ks_import_payment_split_enabled()
{
    return KS_IMPORT_PAYMENT_SPLIT_ENABLED;
}

function ks_import_payment_split_table_exists()
{
    $sql = "SELECT 1 FROM information_schema.tables
        WHERE table_schema=DATABASE() AND table_name=".
        db_escape(TB_PREF.'ks_import_payment_splits');
    $res = db_query($sql);
    return db_num_rows($res) > 0;
}

function ks_import_payment_split_defaults()
{
    return array(
        'forwarder_id' => 0,
        'customs_other' => 0,
        'transport_supplier_id' => 0,
        'transport_amount' => 0
    );
}

function ks_import_payment_split_post_map()
{
    return array(
        'forwarder_id' => 'ks_forwarder_id',
        'customs_other' => 'ks_customs_other',
        'transport_supplier_id' => 'ks_transport_supplier_id',
        'transport_amount' => 'ks_transport_amount'
    );
}

function ks_import_payment_split_post_data()
{
    return array(
        'forwarder_id' => (int)get_post('ks_forwarder_id'),
        'customs_other' => input_num('ks_customs_other', 0),
        'transport_supplier_id' => (int)get_post('ks_transport_supplier_id'),
        'transport_amount' => input_num('ks_transport_amount', 0)
    );
}

function ks_import_forwarder_total($data)
{
    return round((float)$data['customs_duty'] + (float)$data['excise'] +
        (float)$data['import_vat'] + (float)$data['customs_other'], 2);
}

function ks_import_payment_split_validate($data)
{
    if (!ks_import_payment_split_enabled()) {
        return true;
    }
    $ok = true;
    if (!ks_import_payment_split_table_exists()) {
        display_error(_('Tabela e ndarjes se pagesave te importit nuk eshte aktivizuar.'));
        return false;
    }
    foreach (array('customs_other', 'transport_amount') as $field) {
        if ((float)$data[$field] < 0) {
            display_error(_('Shpenzimet e pagesave te importit nuk mund te jene negative.'));
            $ok = false;
            break;
        }
    }
    if (ks_import_forwarder_total($data) > 0 && !(int)$data['forwarder_id']) {
        display_error(_('Zgjidhni shpediterin qe ka paguar procedurat doganore.'));
        set_focus('ks_forwarder_id');
        $ok = false;
    }
    if ((float)$data['transport_amount'] > 0 &&
        !(int)$data['transport_supplier_id']) {
        display_error(_('Zgjidhni transportuesin per shpenzimin e transportit.'));
        set_focus('ks_transport_supplier_id');
        $ok = false;
    }
    if ((int)$data['forwarder_id'] && (int)$data['transport_supplier_id'] &&
        (int)$data['forwarder_id'] == (int)$data['transport_supplier_id']) {
        display_error(_('Shpediteri dhe transportuesi duhet te jene pale te ndara.'));
        $ok = false;
    }
    return $ok;
}

function ks_import_save_payment_split($trans_no, $supplier_id, $data)
{
    if (!ks_import_payment_split_enabled() ||
        !ks_import_payment_split_table_exists()) {
        return false;
    }
    $sql = "SELECT ABS(t.ov_amount) goods_amount, s.curr_code goods_currency
        FROM ".TB_PREF."supp_trans t
        LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=t.supplier_id
        WHERE t.type=".ST_SUPPINVOICE." AND t.trans_no=".(int)$trans_no.
        " AND t.supplier_id=".(int)$supplier_id;
    $res = db_query($sql, _('Detyrimi i furnitorit nuk mund te lexohet.'));
    $invoice = db_fetch_assoc($res);
    if (!$invoice) {
        return false;
    }
    $now = db_escape(date('Y-m-d H:i:s'));
    $user = db_escape(ks_import_user());
    $forwarder_total = ks_import_forwarder_total($data);
    $company_currency = db_escape(get_company_currency());
    $sql = "INSERT INTO ".TB_PREF."ks_import_payment_splits
        (import_trans_type, import_trans_no, goods_supplier_id, goods_amount,
         goods_currency, forwarder_id, customs_duty, excise, import_vat,
         customs_other, forwarder_amount, transport_supplier_id,
         transport_amount, company_currency, status, created_by, created_at,
         updated_by, updated_at)
        VALUES (".ST_SUPPINVOICE.",".(int)$trans_no.",".(int)$supplier_id.",".
        (float)$invoice['goods_amount'].",".db_escape($invoice['goods_currency']).",".
        (int)$data['forwarder_id'].",".(float)$data['customs_duty'].",".
        (float)$data['excise'].",".(float)$data['import_vat'].",".
        (float)$data['customs_other'].",".(float)$forwarder_total.",".
        (int)$data['transport_supplier_id'].",".(float)$data['transport_amount'].",".
        $company_currency.",'draft',".$user.",".$now.",".$user.",".$now.")
        ON DUPLICATE KEY UPDATE goods_supplier_id=VALUES(goods_supplier_id),
        goods_amount=VALUES(goods_amount), goods_currency=VALUES(goods_currency),
        forwarder_id=VALUES(forwarder_id), customs_duty=VALUES(customs_duty),
        excise=VALUES(excise), import_vat=VALUES(import_vat),
        customs_other=VALUES(customs_other), forwarder_amount=VALUES(forwarder_amount),
        transport_supplier_id=VALUES(transport_supplier_id),
        transport_amount=VALUES(transport_amount),
        company_currency=VALUES(company_currency),
        updated_by=VALUES(updated_by), updated_at=VALUES(updated_at)";
    db_query($sql, _('Ndarja e pagesave te importit nuk mund te ruhet.'));
    return true;
}

function ks_import_get_payment_split($trans_no)
{
    if (!ks_import_payment_split_enabled() ||
        !ks_import_payment_split_table_exists()) {
        return false;
    }
    $sql = "SELECT p.*, f.supp_name forwarder_name,
        tr.supp_name transport_supplier_name
        FROM ".TB_PREF."ks_import_payment_splits p
        LEFT JOIN ".TB_PREF."suppliers f ON f.supplier_id=p.forwarder_id
        LEFT JOIN ".TB_PREF."suppliers tr
            ON tr.supplier_id=p.transport_supplier_id
        WHERE p.import_trans_type=".ST_SUPPINVOICE.
        " AND p.import_trans_no=".(int)$trans_no;
    $res = db_query($sql, _('Ndarja e pagesave te importit nuk mund te lexohet.'));
    return db_fetch_assoc($res);
}

function ks_import_payment_split_display_fields()
{
    if (!ks_import_payment_split_enabled()) {
        return;
    }
    if (!ks_import_payment_split_table_exists()) {
        display_warning(_('Aktivizoni perditesimin e modulit per ndarjen e pagesave.'));
        return;
    }
    $data = array_merge(ks_import_payment_split_defaults(),
        ks_import_payment_split_post_data(), ks_import_post_data());
    table_section_title(_('Ndarja automatike e detyrimeve'));
    supplier_list_row(_('Shpediteri:'), 'ks_forwarder_id', null, true, false);
    amount_row(_('Shpenzime tjera doganore:'), 'ks_customs_other');
    label_row(_('Detyrimi i shpediterit:'),
        price_format(ks_import_forwarder_total($data)).' '.get_company_currency());
    supplier_list_row(_('Transportuesi:'), 'ks_transport_supplier_id', null, true, false);
    amount_row(_('Transporti:'), 'ks_transport_amount');
    label_row(_('Statusi i ndarjes:'),
        _('Draft pa pagese bankare; transporti mbahet jashte shpediterit.'));
}
