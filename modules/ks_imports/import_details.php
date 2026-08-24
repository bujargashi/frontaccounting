<?php
$page_security = 'SA_SUPPLIERINVOICE';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/reporting/includes/reporting.inc');
include_once($path_to_root.'/modules/ks_imports/includes/native_import_db.inc');

$trans_no = (int)get_post('trans_no', isset($_GET['trans_no']) ? $_GET['trans_no'] : 0);
$js = user_use_date_picker() ? get_js_date_picker() : '';
page(_($help_context = 'Detajet e Blerjes nga Importi'), false, false, '', $js);
$row = ks_import_get_document($trans_no);
if (!$row) {
    display_error(_('Blerja nga importi nuk u gjet.'));
    end_page();
    exit;
}
$split = ks_import_get_payment_split($trans_no);
$show_split_confirmation = isset($_POST['prepare_liability_split']);

if (isset($_POST['cancel_liability_split'])) {
    if (ks_import_cancel_payment_split($trans_no)) {
        display_notification(_('Drafti u anulua pa prekur faturen ose kontabilitetin.'));
        $split = ks_import_get_payment_split($trans_no);
    }
}

if (isset($_POST['confirm_liability_split'])) {
    if (ks_import_confirm_payment_split($trans_no)) {
        display_notification(_('Detyrimet u ndane dhe u krijuan dokumentet standarde te furnitoreve.'));
        $split = ks_import_get_payment_split($trans_no);
    }
}

if (isset($_POST['update_import'])) {
    $data = ks_import_post_data();
    if (ks_import_validate_data($data)) {
        ks_import_save_document($trans_no, $row['supplier_id'], $data);
        ks_import_save_payment_split($trans_no, $row['supplier_id'], $data);
        display_notification(_('Te dhenat e importit u perditesuan.'));
        $row = ks_import_get_document($trans_no);
        $split = ks_import_get_payment_split($trans_no);
    }
}

$map = array(
    'dud_no' => 'ks_dud_no', 'dud_date' => 'ks_dud_date',
    'clearance_date' => 'ks_clearance_date', 'customs_office' => 'ks_customs_office',
    'origin_country' => 'ks_origin_country', 'incoterm' => 'ks_incoterm',
    'customs_exchange_rate' => 'ks_customs_exchange_rate',
    'tariff_codes' => 'ks_tariff_codes', 'customs_value' => 'ks_customs_value',
    'customs_duty' => 'ks_customs_duty', 'excise' => 'ks_excise',
    'import_vat_base' => 'ks_import_vat_base', 'import_vat' => 'ks_import_vat',
    'eur1_reference' => 'ks_eur1_reference', 'import_notes' => 'ks_import_notes'
 ) + ks_import_payment_split_post_map();
$form_data = array_merge(ks_import_payment_split_defaults(), $row,
    $split ? $split : array());
foreach ($map as $field => $post_name) {
    if (!isset($_POST[$post_name])) {
        $_POST[$post_name] = in_array($field, array('dud_date', 'clearance_date'))
            ? sql2date($form_data[$field]) : $form_data[$field];
    }
}

display_heading(_('BLERJE NGA IMPORTI'));
start_table(TABLESTYLE2);
label_row(_('Furnitori:'), $row['supp_name']);
label_row(_('Fatura e furnitorit:'), $row['supp_reference']);
label_row(_('Data e fatures:'), sql2date($row['tran_date']));
label_row(_('Dokumenti FrontAccounting:'),
    get_trans_view_str(ST_SUPPINVOICE, $trans_no, $row['reference']));
end_table(1);

start_form();
hidden('trans_no', $trans_no);
start_outer_table(TABLESTYLE2);
table_section(1);
table_section_title(_('Dokumenti doganor'));
text_row(_('Numri i DUD-it:'), 'ks_dud_no', null, 28, 80);
date_row(_('Data e DUD-it:'), 'ks_dud_date');
date_row(_('Data e zhdoganimit:'), 'ks_clearance_date');
text_row(_('Zyra doganore:'), 'ks_customs_office', null, 28, 120);
text_row(_('Shteti i origjines:'), 'ks_origin_country', null, 24, 80);
text_row(_('Incoterm:'), 'ks_incoterm', null, 12, 20);
text_row(_('EUR.1 / deshmia e origjines:'), 'ks_eur1_reference', null, 28, 80);
textarea_row(_('Kodet tarifore:'), 'ks_tariff_codes', null, 34, 3, 1000);

table_section(2);
table_section_title(_('Vlerat sipas DUD-it'));
amount_row(_('Kursi doganor:'), 'ks_customs_exchange_rate', null, null, null, 6);
amount_row(_('Vlera doganore:'), 'ks_customs_value');
amount_row(_('Dogana:'), 'ks_customs_duty');
amount_row(_('Akciza:'), 'ks_excise');
amount_row(_('Baza e TVSH-se ne import:'), 'ks_import_vat_base');
amount_row(_('TVSH ne import:'), 'ks_import_vat');
textarea_row(_('Shenime per importin:'), 'ks_import_notes', null, 34, 4, 1000);
ks_import_payment_split_display_fields($split);
end_outer_table(1);
submit_center('update_import', _('Perditeso te dhenat e importit'), true, '', 'default');
end_form();

if ($split) {
    display_heading(_('Drafti i ndarjes se detyrimeve'));
    start_table(TABLESTYLE2);
    label_row(_('Furnitori i mallit:'), $row['supp_name']);
    label_row(_('Detyrimi i furnitorit para avanseve:'),
        price_format($split['goods_amount']).' '.$split['goods_currency']);
    label_row(_('Shpediteri:'), $split['forwarder_name']);
    label_row(_('Detyrimi i shpediterit:'),
        price_format($split['forwarder_amount']).' '.$split['company_currency']);
    label_row(_('Transportuesi:'), $split['transport_supplier_name']);
    label_row(_('Detyrimi i transportuesit:'),
        price_format($split['transport_amount']).' '.$split['company_currency']);
    if ($split['status'] == 'confirmed') {
        label_row(_('Statusi:'), _('E konfirmuar - detyrimet jane ndare.'));
        label_row(_('Kredit-nota e furnitorit:'),
            get_trans_view_str($split['goods_credit_type'],
                $split['goods_credit_no']));
        if ($split['forwarder_invoice_no']) {
            label_row(_('Fatura e detyrimit ndaj shpediterit:'),
                get_trans_view_str($split['forwarder_invoice_type'],
                    $split['forwarder_invoice_no']));
        }
        if ($split['transport_invoice_no']) {
            label_row(_('Fatura e transportuesit:'),
                get_trans_view_str($split['transport_invoice_type'],
                    $split['transport_invoice_no']));
        }
    } elseif ($split['status'] == 'cancelled') {
        label_row(_('Statusi:'),
            _('I anuluar - perditesimi i te dhenave e rikrijon draftin.'));
    } else {
        label_row(_('Statusi:'),
            _('Draft - nuk eshte krijuar pagese ose dokument kontabel.'));
    }
    end_table(1);

    if ($split['status'] == 'draft') {
        start_form();
        hidden('trans_no', $trans_no);
        if ($show_split_confirmation) {
            display_warning(_('Konfirmimi krijon nje kredit-note per furnitorin e mallit dhe faturat per shpediterin/transportuesin. Stoku, kostoja dhe TVSH-ja e blerjes nuk postohen perseri.'));
            submit_center('confirm_liability_split',
                _('Po, krijo dokumentet e ndara'), true, '', 'default');
        } else {
            start_table(TABLESTYLE_NOBORDER);
            start_row();
            submit_cells('prepare_liability_split',
                _('Konfirmo ndarjen e detyrimeve'), true,
                _('Hapi i pare: kontrolloni shumat para krijimit te dokumenteve.'),
                'default');
            submit_cells('cancel_liability_split', _('Anulo draftin'), true,
                _('Anulon vetem draftin; fatura dhe kontabiliteti nuk preken.'),
                ICON_DELETE);
            end_row();
            end_table();
        }
        end_form();
    }
}

start_table(TABLESTYLE_NOBORDER);
start_row();
label_cell("<a class='button' href='../../admin/attachments.php?filterType=".
    ST_SUPPINVOICE.'&trans_no='.$trans_no."'>"._('Dokumentet / Attachment-et').'</a>');
label_cell("<a class='button' href='../../purchasing/supplier_payment.php?trans_type=".
    ST_SUPPINVOICE.'&PInvoice='.$trans_no."'>"._('Pagesa e furnitorit').'</a>');
if ($split && $split['status'] == 'confirmed' && $split['forwarder_invoice_no']) {
    label_cell("<a class='button' href='../../purchasing/supplier_payment.php?trans_type=".
        ST_SUPPINVOICE.'&PInvoice='.$split['forwarder_invoice_no']."'>".
        _('Pagesa e shpediterit').'</a>');
}
if ($split && $split['status'] == 'confirmed' && $split['transport_invoice_no']) {
    label_cell("<a class='button' href='../../purchasing/supplier_payment.php?trans_type=".
        ST_SUPPINVOICE.'&PInvoice='.$split['transport_invoice_no']."'>".
        _('Pagesa e transportuesit').'</a>');
}
label_cell("<a class='button' href='imports.php'>"._('Kthehu ne regjister').'</a>');
end_row();
end_table();
end_page();
