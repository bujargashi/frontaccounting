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

if (isset($_POST['update_import'])) {
    $data = ks_import_post_data();
    if (ks_import_validate_data($data)) {
        ks_import_save_document($trans_no, $row['supplier_id'], $data);
        display_notification(_('Te dhenat e importit u perditesuan.'));
        $row = ks_import_get_document($trans_no);
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
);
foreach ($map as $field => $post_name) {
    if (!isset($_POST[$post_name])) {
        $_POST[$post_name] = in_array($field, array('dud_date', 'clearance_date'))
            ? sql2date($row[$field]) : $row[$field];
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
end_outer_table(1);
submit_center('update_import', _('Perditeso te dhenat e importit'), true, '', 'default');
end_form();

start_table(TABLESTYLE_NOBORDER);
start_row();
label_cell("<a class='button' href='../../admin/attachments.php?filterType=".
    ST_SUPPINVOICE.'&trans_no='.$trans_no."'>"._('Dokumentet / Attachment-et').'</a>');
label_cell("<a class='button' href='../../purchasing/supplier_payment.php?trans_type=".
    ST_SUPPINVOICE.'&PInvoice='.$trans_no."'>"._('Pagesa e furnitorit').'</a>');
label_cell("<a class='button' href='imports.php'>"._('Kthehu ne regjister').'</a>');
end_row();
end_table();
end_page();
