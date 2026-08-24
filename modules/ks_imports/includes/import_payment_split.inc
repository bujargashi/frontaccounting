<?php

include_once($path_to_root.'/purchasing/includes/purchasing_db.inc');

if (!defined('KS_IMPORT_PAYMENT_SPLIT_ENABLED')) {
    define('KS_IMPORT_PAYMENT_SPLIT_ENABLED', true);
}

function ks_import_payment_split_enabled()
{
    return KS_IMPORT_PAYMENT_SPLIT_ENABLED;
}

function ks_import_payment_split_table_exists()
{
    $sql = "SELECT COUNT(*) FROM ".TB_PREF."ks_import_payment_splits";
    $res = db_query($sql);
    return $res !== false;
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

function ks_import_third_party_total($data)
{
    return round(ks_import_forwarder_total($data) +
        (float)$data['transport_amount'], 2);
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
        display_error(_('Zgjidhni transportuesin kur transporti paguhet vecmas.'));
        set_focus('ks_transport_supplier_id');
        $ok = false;
    }
    if ((int)$data['forwarder_id'] && (int)$data['transport_supplier_id'] &&
        (int)$data['forwarder_id'] == (int)$data['transport_supplier_id']) {
        display_error(_('Shpediteri dhe transportuesi duhet te jene pale te ndara.'));
        $ok = false;
    }
    if (isset($_SESSION['PO']) && method_exists($_SESSION['PO'], 'get_trans_total')) {
        $invoice_home = (float)$_SESSION['PO']->get_trans_total() *
            (float)$_SESSION['PO']->ex_rate;
        if (ks_import_third_party_total($data) > round($invoice_home, 2)) {
            display_error(_('Detyrimet ndaj shpediterit dhe transportuesit nuk mund te kalojne totalin e blerjes.'));
            $ok = false;
        }
    }
    return $ok;
}

function ks_import_get_source_invoice($trans_no, $supplier_id)
{
    $sql = "SELECT ABS(t.ov_amount+t.ov_gst+t.ov_discount) invoice_total,
        t.rate, t.tran_date, t.due_date, t.reference, t.supp_reference,
        s.curr_code goods_currency, s.supp_name
        FROM ".TB_PREF."supp_trans t
        LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=t.supplier_id
        WHERE t.type=".ST_SUPPINVOICE." AND t.trans_no=".(int)$trans_no.
        " AND t.supplier_id=".(int)$supplier_id;
    $res = db_query($sql, _('Fatura e furnitorit nuk mund te lexohet.'));
    return db_fetch_assoc($res);
}

function ks_import_save_payment_split($trans_no, $supplier_id, $data)
{
    if (!ks_import_payment_split_enabled() ||
        !ks_import_payment_split_table_exists()) {
        return false;
    }
    $existing = ks_import_get_payment_split($trans_no);
    if ($existing && $existing['status'] == 'confirmed') {
        return true;
    }
    $invoice = ks_import_get_source_invoice($trans_no, $supplier_id);
    if (!$invoice) {
        return false;
    }
    $forwarder_total = ks_import_forwarder_total($data);
    $third_party_home = $forwarder_total + (float)$data['transport_amount'];
    $rate = (float)$invoice['rate'] > 0 ? (float)$invoice['rate'] : 1;
    $third_party_supplier = round2($third_party_home / $rate, user_price_dec());
    $goods_amount = round2((float)$invoice['invoice_total'] -
        $third_party_supplier, user_price_dec());
    if ($goods_amount < 0) {
        display_error(_('Ndarja eshte me e madhe se totali i fatures se blerjes.'));
        return false;
    }
    $now = db_escape(date('Y-m-d H:i:s'));
    $user = db_escape(ks_import_user());
    $company_currency = db_escape(get_company_currency());
    $sql = "INSERT INTO ".TB_PREF."ks_import_payment_splits
        (import_trans_type, import_trans_no, goods_supplier_id, goods_amount,
         goods_currency, forwarder_id, customs_duty, excise, import_vat,
         customs_other, forwarder_amount, transport_supplier_id,
         transport_amount, company_currency, status, created_by, created_at,
         updated_by, updated_at)
        VALUES (".ST_SUPPINVOICE.",".(int)$trans_no.",".(int)$supplier_id.",".
        (float)$goods_amount.",".db_escape($invoice['goods_currency']).",".
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
        company_currency=VALUES(company_currency), status='draft',
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

function ks_import_create_liability_document($type, $supplier_id, $amount,
    $date_, $supp_reference, $memo, $source_invoice=0)
{
    global $Refs;

    $supplier = get_supplier($supplier_id);
    $trans = new supp_trans($type);
    $trans->supplier_id = $supplier_id;
    $trans->tran_date = $date_;
    $trans->due_date = $date_;
    $trans->dimension = $supplier['dimension_id'];
    $trans->dimension2 = $supplier['dimension2_id'];
    $trans->reference = $Refs->get_next($type, null,
        array('supplier'=>$supplier_id, 'date'=>$date_));
    $trans->supp_reference = $supp_reference;
    $trans->tax_included = 0;
    $trans->tax_group_id = $supplier['tax_group_id'];
    $trans->currency = $supplier['curr_code'];
    $trans->ex_rate = get_exchange_rate_from_home_currency(
        $supplier['curr_code'], $date_);
    $trans->ov_amount = $trans->ov_gst = $trans->ov_discount = 0;
    $trans->Comments = $memo;
    $trans->add_gl_codes_to_trans($supplier['payable_account'],
        _('Riklasifikim i detyrimit te importit'), 0, 0, $amount, $memo);
    if ($type == ST_SUPPCREDIT && $source_invoice) {
        $trans->src_docs = array($source_invoice => true);
    }
    return add_supp_invoice($trans);
}

function ks_import_confirm_payment_split($trans_no)
{
    $split = ks_import_get_payment_split($trans_no);
    if (!$split || $split['status'] != 'draft') {
        display_error(_('Vetem nje draft mund te konfirmohet.'));
        return false;
    }
    $document = ks_import_get_document($trans_no);
    $invoice = ks_import_get_source_invoice($trans_no,
        $split['goods_supplier_id']);
    if (!$document || !$invoice) {
        return false;
    }
    $third_party_home = (float)$split['forwarder_amount'] +
        (float)$split['transport_amount'];
    if ($third_party_home <= 0) {
        display_error(_('Nuk ka detyrime ndaj paleve te treta per t\'u ndare.'));
        return false;
    }
    $rate = (float)$invoice['rate'] > 0 ? (float)$invoice['rate'] : 1;
    $goods_credit_amount = round2($third_party_home / $rate,
        user_price_dec());
    if ($goods_credit_amount > (float)$invoice['invoice_total']) {
        display_error(_('Ndarja eshte me e madhe se totali i fatures se blerjes.'));
        return false;
    }
    $invoice_date = sql2date($invoice['tran_date']);
    $clearance_date = sql2date($document['clearance_date']);
    $memo = _('Ndarja e detyrimeve per importin').' #'.$trans_no;
    $supp_reference = 'IMPORT-'.$trans_no;

    begin_transaction();
    $goods_credit_no = ks_import_create_liability_document(ST_SUPPCREDIT,
        $split['goods_supplier_id'], $goods_credit_amount, $invoice_date,
        $supp_reference, $memo, $trans_no);
    $forwarder_invoice_no = 0;
    if ((float)$split['forwarder_amount'] > 0) {
        $forwarder_currency = get_supplier_currency($split['forwarder_id']);
        $forwarder_amount = round2(exchange_from_to(
            $split['forwarder_amount'], get_company_currency(),
            $forwarder_currency, $clearance_date), user_price_dec());
        $forwarder_invoice_no = ks_import_create_liability_document(
            ST_SUPPINVOICE, $split['forwarder_id'], $forwarder_amount,
            $clearance_date, $supp_reference, $memo);
    }
    $transport_invoice_no = 0;
    if ((float)$split['transport_amount'] > 0) {
        $transport_currency = get_supplier_currency(
            $split['transport_supplier_id']);
        $transport_amount = round2(exchange_from_to(
            $split['transport_amount'], get_company_currency(),
            $transport_currency, $clearance_date), user_price_dec());
        $transport_invoice_no = ks_import_create_liability_document(
            ST_SUPPINVOICE, $split['transport_supplier_id'], $transport_amount,
            $clearance_date, $supp_reference, $memo);
    }
    $sql = "UPDATE ".TB_PREF."ks_import_payment_splits SET
        status='confirmed', goods_credit_type=".ST_SUPPCREDIT.",
        goods_credit_no=".(int)$goods_credit_no.",
        forwarder_invoice_type=".ST_SUPPINVOICE.",
        forwarder_invoice_no=".(int)$forwarder_invoice_no.",
        transport_invoice_type=".ST_SUPPINVOICE.",
        transport_invoice_no=".(int)$transport_invoice_no.",
        confirmed_by=".db_escape(ks_import_user()).",
        confirmed_at=".db_escape(date('Y-m-d H:i:s')).",
        updated_by=".db_escape(ks_import_user()).",
        updated_at=".db_escape(date('Y-m-d H:i:s'))."
        WHERE import_trans_type=".ST_SUPPINVOICE.
        " AND import_trans_no=".(int)$trans_no." AND status='draft'";
    db_query($sql, _('Ndarja e detyrimeve nuk mund te konfirmohet.'));
    commit_transaction();
    return true;
}

function ks_import_cancel_payment_split($trans_no)
{
    $split = ks_import_get_payment_split($trans_no);
    if (!$split || $split['status'] != 'draft') {
        display_error(_('Vetem drafti i pakonfirmuar mund te anulohet.'));
        return false;
    }
    $sql = "UPDATE ".TB_PREF."ks_import_payment_splits SET status='cancelled',
        cancelled_by=".db_escape(ks_import_user()).",
        cancelled_at=".db_escape(date('Y-m-d H:i:s')).",
        updated_by=".db_escape(ks_import_user()).",
        updated_at=".db_escape(date('Y-m-d H:i:s'))."
        WHERE import_trans_type=".ST_SUPPINVOICE.
        " AND import_trans_no=".(int)$trans_no." AND status='draft'";
    db_query($sql, _('Drafti i ndarjes nuk mund te anulohet.'));
    return true;
}

function ks_import_payment_split_display_fields($saved_split=false)
{
    if (!ks_import_payment_split_enabled()) {
        return;
    }
    if (!ks_import_payment_split_table_exists()) {
        display_warning(_('Aktivizoni perditesimin e modulit per ndarjen e pagesave.'));
        return;
    }
    $locked = $saved_split && $saved_split['status'] == 'confirmed';
    $data = array_merge(ks_import_payment_split_defaults(),
        ks_import_payment_split_post_data(), ks_import_post_data(),
        $locked ? $saved_split : array());
    table_section_title(_('Ndarja automatike e detyrimeve'));
    if ($locked) {
        label_row(_('Shpediteri:'), $saved_split['forwarder_name']);
        label_row(_('Shpenzime tjera doganore:'),
            price_format($saved_split['customs_other']));
        label_row(_('Detyrimi i shpediterit:'),
            price_format($saved_split['forwarder_amount']).' '.get_company_currency());
        label_row(_('Transportuesi:'), $saved_split['transport_supplier_name']);
        label_row(_('Transporti i paguar vecmas:'),
            price_format($saved_split['transport_amount']).' '.get_company_currency());
        hidden('ks_forwarder_id', $saved_split['forwarder_id']);
        hidden('ks_customs_other', $saved_split['customs_other']);
        hidden('ks_transport_supplier_id', $saved_split['transport_supplier_id']);
        hidden('ks_transport_amount', $saved_split['transport_amount']);
    } else {
        supplier_list_row(_('Shpediteri:'), 'ks_forwarder_id', null, true, false);
        amount_row(_('Shpenzime tjera doganore:'), 'ks_customs_other');
        label_row(_('Detyrimi i shpediterit:'),
            price_format(ks_import_forwarder_total($data)).' '.get_company_currency());
        supplier_list_row(_('Transportuesi (vetem kur paguhet vecmas):'),
            'ks_transport_supplier_id', null, true, false);
        amount_row(_('Transporti i paguar vecmas:'), 'ks_transport_amount');
    }
    label_row(_('Statusi i ndarjes:'), $locked
        ? _('E konfirmuar - pagesat behen me Payment to Supplier.')
        : _('Draft pa pagese bankare.'));
}
