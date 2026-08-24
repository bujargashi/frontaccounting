<?php

$page_security = 'SA_KS_IMPORT_VIEW';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/modules/ks_imports/includes/db.inc');

$js = user_use_date_picker() ? get_js_date_picker() : '';
page(_($help_context = 'Regjistri i importeve - Kosove'), false, false, '', $js);

$delete_id = find_submit('DeleteImport', false);
if ($delete_id !== null) {
    if (ks_import_delete($delete_id)) {
        display_notification(_('Drafti i importit u fshi.'));
    } else {
        display_error(_('Mund te fshihen vetem dosjet Draft.'));
    }
}

if (!isset($_POST['from_date'])) {
    $_POST['from_date'] = begin_month(Today());
    $_POST['to_date'] = Today();
    $_POST['status_filter'] = '';
    $_POST['search'] = '';
}

start_form();
start_table(TABLESTYLE_NOBORDER);
start_row();
date_cells(_('Nga:'), 'from_date');
date_cells(_('Deri:'), 'to_date');
label_cell(_('Statusi:'));
echo '<td>'.array_selector('status_filter', null, array(
    '' => _('Te gjitha'), 'draft' => _('Draft'), 'ready' => _('Gati'),
    'posted' => _('Postuar'), 'void' => _('Anuluar')
)).'</td>';
text_cells(_('Kerko:'), 'search', null, 28, 80);
submit_cells('filter', _('Filtro'), '', '', 'default');
end_row();
end_table();
end_form();

$where = array(
    "COALESCE(i.dud_date, i.supplier_invoice_date) >= ".db_escape(date2sql(get_post('from_date'))),
    "COALESCE(i.dud_date, i.supplier_invoice_date) <= ".db_escape(date2sql(get_post('to_date')))
);
if (get_post('status_filter') !== '') {
    $where[] = 'i.status='.db_escape(get_post('status_filter'));
}
if (trim(get_post('search')) !== '') {
    $needle = '%'.trim(get_post('search')).'%';
    $where[] = '(i.reference LIKE '.db_escape($needle).' OR i.dud_no LIKE '.db_escape($needle).
        ' OR i.supplier_invoice_no LIKE '.db_escape($needle).' OR s.supp_name LIKE '.db_escape($needle).')';
}

$sql = "SELECT i.*, s.supp_name,
        (SELECT COUNT(*) FROM ".TB_PREF."ks_import_items d WHERE d.import_id=i.id) item_count,
        (SELECT SUM(d.landed_cost) FROM ".TB_PREF."ks_import_items d WHERE d.import_id=i.id) landed_total
    FROM ".TB_PREF."ks_imports i
    LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=i.supplier_id
    WHERE ".implode(' AND ', $where)." ORDER BY COALESCE(i.dud_date, i.supplier_invoice_date) DESC, i.id DESC";
$result = db_query($sql, _('Regjistri nuk mund te lexohet.'));

$status_labels = array('draft' => _('Draft'), 'ready' => _('Gati'), 'posted' => _('Postuar'), 'void' => _('Anuluar'));
display_heading(_('Regjistri i importeve'));
start_form();
start_table(TABLESTYLE, "width='98%'");
table_header(array(_('Referenca'), _('Furnitori'), _('Fatura'), _('DUD'), _('Data DUD'),
    _('Origjina'), _('Artikuj'), _('Vlera doganore'), _('TVSH import'), _('Landed cost'), _('Statusi'), '', ''));
$k = 0;
while ($row = db_fetch_assoc($result)) {
    alt_table_row_color($k);
    label_cell("<a href='import_entry.php?id=".(int)$row['id']."'>".$row['reference']."</a>");
    label_cell($row['supp_name']);
    label_cell($row['supplier_invoice_no']);
    label_cell($row['dud_no']);
    label_cell($row['dud_date'] ? sql2date($row['dud_date']) : '');
    label_cell($row['origin_country']);
    qty_cell($row['item_count'], false, 0);
    amount_cell($row['customs_value_base']);
    amount_cell($row['import_vat_amount']);
    amount_cell($row['landed_total']);
    label_cell(@$status_labels[$row['status']]);
    label_cell("<a href='import_entry.php?id=".(int)$row['id']."'>"._('Hap')."</a>");
    if ($row['status'] == 'draft') {
        delete_button_cell('DeleteImport'.$row['id'], _('Fshi'));
        submit_js_confirm('DeleteImport'.$row['id'], _('A jeni i sigurt se doni ta fshini kete draft?'));
    } else {
        label_cell('');
    }
    end_row();
}
end_table(1);
end_form();

start_table(TABLESTYLE_NOBORDER);
start_row();
label_cell("<a class='button' href='import_entry.php'>"._('Import i ri')."</a>");
label_cell("<a class='button' href='export_purchase_book.php?from=".
    urlencode(get_post('from_date'))."&to=".urlencode(get_post('to_date'))."'>".
    _('Eksporto regjistrin CSV')."</a>");
end_row();
end_table();

end_page();
