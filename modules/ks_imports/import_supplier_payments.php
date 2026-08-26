<?php
$page_security = 'SA_SUPPLIERPAYMNT';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/reporting/includes/reporting.inc');
include_once($path_to_root.
    '/modules/ks_imports/includes/import_supplier_payments_db.inc');

page(_($help_context = 'Pagesat e Furnitoreve nga Importi'));

if (!isset($_POST['supplier_id'])) {
    $_POST['supplier_id'] = 0;
}

start_form();
start_table(TABLESTYLE_NOBORDER);
start_row();
supplier_list_cells(_('Furnitori:'), 'supplier_id', null,
    _('Te gjithe furnitoret'), true);
submit_cells('filter', _('Filtro'), '', '', 'default');
end_row();
end_table();
end_form();

display_heading(_('DETYRIMET NETO NDAJ FURNITOREVE TE IMPORTIT'));
display_note(_('Shumat jane pa TVSH. Pagesa dhe alokimi kryhen nga funksioni standard i FrontAccounting.'), 0, 1);

start_table(TABLESTYLE, "width='99%'");
table_header(array(
    _('Data'), _('Afati'), _('Furnitori'), _('Fatura'), _('DUD'),
    _('Monedha'), _('Vlera neto'), _('Alokuar'), _('Obligimi neto'), ''
));

$res = ks_import_supplier_obligations_query((int)get_post('supplier_id'));
$k = 0;
while ($row = db_fetch_assoc($res)) {
    alt_table_row_color($k);
    label_cell(sql2date($row['tran_date']));
    label_cell(sql2date($row['due_date']));
    label_cell($row['supp_name']);
    label_cell(get_trans_view_str(ST_SUPPINVOICE, $row['trans_no'],
        $row['supp_reference']));
    label_cell($row['dud_no']);
    label_cell($row['curr_code']);
    amount_cell($row['net_total']);
    amount_cell(min($row['alloc'], $row['net_total']));
    amount_cell($row['net_due']);
    label_cell("<a class='button' href='../../purchasing/supplier_payment.php?".
        "supplier_id=".(int)$row['supplier_id'].
        "&trans_type=".ST_SUPPINVOICE.
        "&PInvoice=".(int)$row['trans_no']."'>".
        _('Paguaj').'</a>');
    end_row();
}
end_table(1);

display_note(_('TVSH-ja sipas DUD-it nuk perfshihet ne obligimin ndaj furnitorit te huaj.'), 0, 1);
end_page();
