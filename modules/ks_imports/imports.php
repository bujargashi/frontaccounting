<?php
$page_security = 'SA_SUPPTRANSVIEW';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/reporting/includes/reporting.inc');
include_once($path_to_root.'/modules/ks_imports/includes/native_import_db.inc');

page(_($help_context = 'Regjistri i Blerjeve nga Importi'), false, false, '',
    user_use_date_picker() ? get_js_date_picker() : '');

if (!isset($_POST['from_date'])) {
    $_POST['from_date'] = begin_month(Today());
    $_POST['to_date'] = Today();
    $_POST['supplier_id'] = 0;
    $_POST['search'] = '';
}

start_form();
start_table(TABLESTYLE_NOBORDER);
start_row();
date_cells(_('Nga:'), 'from_date');
date_cells(_('Deri:'), 'to_date');
supplier_list_cells(_('Furnitori:'), 'supplier_id', null, _('Te gjithe furnitoret'), true);
text_cells(_('Kerko:'), 'search', null, 24, 80);
submit_cells('filter', _('Filtro'), '', '', 'default');
end_row();
end_table();
end_form();

display_heading(_('REGJISTRI I BLERJEVE NGA IMPORTI'));
start_table(TABLESTYLE, "width='99%'");
table_header(array(
    _('Data'), _('Furnitori'), _('Fatura'), _('DUD'), _('Origjina'),
    _('Vlera doganore'), _('Dogana'), _('Akciza'), _('Baza TVSH'),
    _('TVSH import'), _('Fatura FA'), ''
));

$res = ks_import_query(get_post('from_date'), get_post('to_date'),
    (int)get_post('supplier_id'), get_post('search'));
$k = 0;
$totals = array('customs_value' => 0, 'customs_duty' => 0, 'excise' => 0,
    'import_vat_base' => 0, 'import_vat' => 0);
while ($row = db_fetch_assoc($res)) {
    alt_table_row_color($k);
    label_cell(sql2date($row['clearance_date']));
    label_cell($row['supp_name']);
    label_cell($row['supp_reference']);
    label_cell($row['dud_no']);
    label_cell($row['origin_country']);
    amount_cell($row['customs_value']);
    amount_cell($row['customs_duty']);
    amount_cell($row['excise']);
    amount_cell($row['import_vat_base']);
    amount_cell($row['import_vat']);
    label_cell(get_trans_view_str(ST_SUPPINVOICE, $row['trans_no'],
        $row['reference']));
    label_cell("<a href='import_details.php?trans_no=".(int)$row['trans_no']."'>".
        _('Detajet').'</a>');
    end_row();
    foreach ($totals as $name => $value) {
        $totals[$name] += $row[$name];
    }
}

start_row();
label_cell('<b>'._('TOTALI').'</b>', "colspan='5'");
foreach ($totals as $value) {
    amount_cell($value, true);
}
label_cell('', "colspan='2'");
end_row();
end_table(1);

start_table(TABLESTYLE_NOBORDER);
start_row();
label_cell("<a class='button' href='purchase_import.php?New=1'>".
    _('Blerje e re nga Importi').'</a>');
label_cell("<a class='button' href='../../reporting/reports_main.php?Class=1'>".
    _('Raportet e Blerjeve').'</a>');
end_row();
end_table();
end_page();
