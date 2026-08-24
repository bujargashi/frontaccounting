<?php

$page_security = 'SA_KS_IMPORT_ENTRY';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/modules/ks_imports/includes/db.inc');
include_once($path_to_root.'/modules/ks_imports/includes/calculations.inc');

$js = user_use_date_picker() ? get_js_date_picker() : '';
page(_($help_context = 'Importi i mallrave - Kosove'), false, false, '', $js);

$import_id = (int)get_post('import_id', get_post('id', 0));
if (isset($_GET['id'])) {
    $import_id = (int)$_GET['id'];
}

function ks_import_post_data()
{
    $data = array(
        'reference' => trim(get_post('reference')),
        'supplier_id' => (int)get_post('supplier_id'),
        'supplier_invoice_no' => trim(get_post('supplier_invoice_no')),
        'supplier_invoice_date' => get_post('supplier_invoice_date'),
        'supplier_trans_no' => (int)get_post('supplier_trans_no'),
        'origin_country' => trim(get_post('origin_country')),
        'dispatch_country' => trim(get_post('dispatch_country')),
        'dud_no' => trim(get_post('dud_no')),
        'dud_date' => get_post('dud_date'),
        'customs_office' => trim(get_post('customs_office')),
        'incoterm' => trim(get_post('incoterm')),
        'currency' => get_post('currency', 'EUR'),
        'exchange_rate' => ks_import_num(get_post('exchange_rate', 1)),
        'goods_amount_currency' => ks_import_num(get_post('goods_amount_currency')),
        'transport_mode' => get_post('transport_mode', 'included'),
        'transport_invoice_no' => trim(get_post('transport_invoice_no')),
        'transport_supplier_id' => (int)get_post('transport_supplier_id'),
        'transport_trans_no' => (int)get_post('transport_trans_no'),
        'transport_amount_base' => ks_import_num(get_post('transport_amount_base')),
        'insurance_amount_base' => ks_import_num(get_post('insurance_amount_base')),
        'handling_amount_base' => ks_import_num(get_post('handling_amount_base')),
        'customs_value_override' => ks_import_num(get_post('customs_value_override')),
        'customs_duty_base' => ks_import_num(get_post('customs_duty_base')),
        'excise_base' => ks_import_num(get_post('excise_base')),
        'terminal_amount_base' => ks_import_num(get_post('terminal_amount_base')),
        'forwarding_amount_base' => ks_import_num(get_post('forwarding_amount_base')),
        'other_amount_base' => ks_import_num(get_post('other_amount_base')),
        'import_vat_rate' => ks_import_num(get_post('import_vat_rate', 18)),
        'import_vat_base_override' => ks_import_num(get_post('import_vat_base_override')),
        'import_vat_amount_override' => ks_import_num(get_post('import_vat_amount_override')),
        'allocation_method' => get_post('allocation_method', 'value'),
        'notes' => trim(get_post('notes'))
    );
    return array_merge($data, ks_import_calculate($data));
}

function ks_import_validate_header($data, $ready = false)
{
    $ok = true;
    if ($data['reference'] === '') {
        display_error(_('Referenca eshte e detyrueshme.'));
        $ok = false;
    }
    if (!$data['supplier_id']) {
        display_error(_('Duhet te zgjidhet furnitori.'));
        $ok = false;
    }
    if ($data['supplier_invoice_no'] === '') {
        display_error(_('Numri i fatures se furnitorit eshte i detyrueshem.'));
        $ok = false;
    }
    if ($data['origin_country'] === '') {
        display_error(_('Shteti i origjines eshte i detyrueshem.'));
        $ok = false;
    }
    if ($data['goods_amount_currency'] <= 0) {
        display_error(_('Vlera e mallit duhet te jete me e madhe se zero.'));
        $ok = false;
    }
    if ($data['exchange_rate'] <= 0) {
        display_error(_('Kursi i kembimit duhet te jete me i madh se zero.'));
        $ok = false;
    }
    if ($ready && ($data['dud_no'] === '' || !$data['dud_date'])) {
        display_error(_('Numri dhe data e DUD-it kerkohen para mbylljes se dosjes.'));
        $ok = false;
    }
    return $ok;
}

if (isset($_POST['save'])) {
    $data = ks_import_post_data();
    if (ks_import_validate_header($data)) {
        if (ks_import_find_by_dud($data['dud_no'], $import_id)) {
            display_error(_('Ky numer DUD eshte regjistruar ne nje import tjeter.'));
        } elseif ($data['supplier_trans_no'] &&
            !ks_import_get_supplier_invoice($data['supplier_id'], $data['supplier_trans_no'])) {
            display_error(_('Fatura e zgjedhur e FA nuk i perket furnitorit te zgjedhur.'));
        } elseif ($data['supplier_trans_no'] &&
            ks_import_invoice_is_linked($data['supplier_id'], $data['supplier_trans_no'], $import_id)) {
            display_error(_('Kjo fature e FA eshte lidhur me nje dosje tjeter importi.'));
        } else {
            begin_transaction();
            $import_id = ks_import_save($import_id, $data);
            $items = ks_import_items($import_id);
            if (count($items)) {
                ks_import_store_allocations($import_id, ks_import_allocate($items, $data));
            }
            commit_transaction();
            display_notification(_('Importi u ruajt me sukses.'));
        }
    }
}

if (isset($_POST['add_item']) && $import_id) {
    $item = array(
        'stock_id' => trim(get_post('new_stock_id')),
        'description' => trim(get_post('new_description')),
        'quantity' => ks_import_num(get_post('new_quantity')),
        'weight' => ks_import_num(get_post('new_weight')),
        'invoice_value_base' => ks_import_num(get_post('new_invoice_value')),
        'manual_share' => ks_import_num(get_post('new_manual_share'))
    );
    if ($item['stock_id'] && $item['description'] === '') {
        $item['description'] = ks_import_stock_description($item['stock_id']);
    }
    if (!ks_import_is_draft($import_id)) {
        display_error(_('Vetem dosja Draft mund te ndryshohet.'));
    } elseif ($item['description'] === '' || $item['quantity'] <= 0 || $item['invoice_value_base'] < 0) {
        display_error(_('Plotesoni pershkrimin, sasine dhe vleren e artikullit.'));
    } else {
        begin_transaction();
        ks_import_add_item($import_id, $item);
        $header = ks_import_get($import_id);
        ks_import_store_allocations($import_id, ks_import_allocate(ks_import_items($import_id), $header));
        commit_transaction();
        unset($_POST['new_stock_id'], $_POST['new_description'], $_POST['new_quantity'],
            $_POST['new_weight'], $_POST['new_invoice_value'], $_POST['new_manual_share']);
        display_notification(_('Artikulli u shtua.'));
    }
}

if (isset($_POST['load_fa_invoice']) && $import_id) {
    $supplier_id = (int)get_post('supplier_id');
    $trans_no = (int)get_post('supplier_trans_no');
    $existing = ks_import_items($import_id);
    $invoice = ks_import_get_supplier_invoice($supplier_id, $trans_no);
    if (!ks_import_is_draft($import_id)) {
        display_error(_('Vetem dosja Draft mund te ndryshohet.'));
    } elseif (!$invoice) {
        display_error(_('Zgjidhni nje fature valide te furnitorit.'));
    } elseif (count($existing)) {
        display_error(_('Fatura nuk u importua sepse dosja tashme ka artikuj. Fshini artikujt ekzistues nese doni ta ngarkoni faturen e FA.'));
    } elseif (ks_import_invoice_is_linked($supplier_id, $trans_no, $import_id)) {
        display_error(_('Kjo fature e FrontAccounting eshte lidhur me nje dosje tjeter importi.'));
    } else {
        $data = ks_import_post_data();
        $data['supplier_invoice_no'] = $invoice['supp_reference'];
        $data['supplier_invoice_date'] = sql2date($invoice['tran_date']);
        $data['supplier_trans_no'] = $invoice['trans_no'];
        $data['currency'] = $invoice['curr_code'];
        $data['exchange_rate'] = (float)$invoice['rate'] > 0 ? round(1 / (float)$invoice['rate'], 8) : 1;
        $data['goods_amount_currency'] = (float)$invoice['ov_amount'];
        $data = array_merge($data, ks_import_calculate($data));
        begin_transaction();
        ks_import_save($import_id, $data);
        $invoice_items = ks_import_get_supplier_invoice_items($trans_no);
        foreach ($invoice_items as $source_item) {
            $qty = (float)$source_item['quantity'];
            ks_import_add_item($import_id, array(
                'stock_id' => $source_item['stock_id'],
                'description' => $source_item['description'] ? $source_item['description'] : ks_import_stock_description($source_item['stock_id']),
                'quantity' => $qty,
                'weight' => 0,
                'invoice_value_base' => round($qty * (float)$source_item['unit_price'] * $data['exchange_rate'], 2),
                'manual_share' => 0
            ));
        }
        ks_import_store_allocations($import_id, ks_import_allocate(ks_import_items($import_id), $data));
        commit_transaction();
        display_notification(_('Fatura dhe artikujt u moren nga FrontAccounting.'));
    }
}

$delete_item = find_submit('DeleteItem', false);
if ($delete_item !== null && $import_id) {
    begin_transaction();
    ks_import_delete_item($import_id, $delete_item);
    $header = ks_import_get($import_id);
    ks_import_store_allocations($import_id, ks_import_allocate(ks_import_items($import_id), $header));
    commit_transaction();
    display_notification(_('Artikulli u fshi.'));
}

if (isset($_POST['mark_ready']) && $import_id) {
    $data = ks_import_post_data();
    $items = ks_import_items($import_id);
    $item_value_total = 0;
    foreach ($items as $check_item) {
        $item_value_total += (float)$check_item['invoice_value_base'];
    }
    if (!count($items)) {
        display_error(_('Duhet te regjistrohet te pakten nje artikull.'));
    } elseif (abs($item_value_total - $data['goods_amount_base']) > 0.02) {
        display_error(sprintf(_('Shuma e artikujve (%s EUR) nuk perputhet me vleren e mallit (%s EUR).'),
            price_format($item_value_total), price_format($data['goods_amount_base'])));
    } elseif (!ks_import_validate_header($data, true)) {
        // Validation messages are displayed by ks_import_validate_header().
    } elseif (ks_import_find_by_dud($data['dud_no'], $import_id)) {
        display_error(_('Ky numer DUD eshte regjistruar ne nje import tjeter.'));
    } else {
        begin_transaction();
        ks_import_save($import_id, $data);
        ks_import_store_allocations($import_id, ks_import_allocate($items, $data));
        ks_import_mark_ready($import_id);
        commit_transaction();
        display_notification(_('Dosja u kontrollua dhe u shenua Gati. Nuk eshte bere postim ne librin kryesor.'));
    }
}

if (isset($_POST['reopen']) && $import_id) {
    ks_import_reopen($import_id);
    display_notification(_('Dosja u rikthye ne Draft.'));
}

$row = $import_id ? ks_import_get($import_id) : false;
if (!$row) {
    $row = array(
        'reference' => ks_import_next_reference(), 'status' => 'draft', 'supplier_id' => 0,
        'supplier_invoice_no' => '', 'supplier_invoice_date' => date2sql(Today()), 'supplier_trans_no' => 0,
        'origin_country' => '', 'dispatch_country' => '', 'dud_no' => '', 'dud_date' => date2sql(Today()),
        'customs_office' => '', 'incoterm' => '', 'currency' => 'EUR', 'exchange_rate' => 1,
        'goods_amount_currency' => 0, 'goods_amount_base' => 0, 'transport_mode' => 'included',
        'transport_invoice_no' => '', 'transport_supplier_id' => 0, 'transport_trans_no' => 0,
        'transport_amount_base' => 0, 'insurance_amount_base' => 0,
        'handling_amount_base' => 0, 'customs_value_base' => 0, 'customs_duty_base' => 0,
        'excise_base' => 0, 'terminal_amount_base' => 0, 'forwarding_amount_base' => 0,
        'other_amount_base' => 0, 'customs_value_override' => 0,
        'import_vat_rate' => ks_import_setting('default_vat_rate', 18),
        'import_vat_base_override' => 0, 'import_vat_amount_override' => 0,
        'import_vat_base' => 0, 'import_vat_amount' => 0, 'allocation_method' => 'value', 'notes' => ''
    );
}

$keep_post_for_selector = isset($_POST['_supplier_id_update']) || isset($_POST['_transport_supplier_id_update']);
if (!$keep_post_for_selector && (!isset($_POST['reference']) || !isset($_POST['save']))) {
    foreach ($row as $key => $value) {
        if (in_array($key, array('supplier_invoice_date', 'dud_date')) && $value) {
            $_POST[$key] = sql2date($value);
        } else {
            $_POST[$key] = $value;
        }
    }
}

$status_labels = array('draft' => _('Draft'), 'ready' => _('Gati'), 'posted' => _('Postuar'), 'void' => _('Anuluar'));
display_heading(sprintf(_('Dosja %s - %s'), $row['reference'], @$status_labels[$row['status']]));

start_form();
hidden('import_id', $import_id);
start_outer_table(TABLESTYLE2);
table_section(1);
table_section_title(_('Dokumenti dhe furnitori'));
text_row(_('Referenca:'), 'reference', null, 24, 40);
supplier_list_row(_('Furnitori:'), 'supplier_id', null, _('Zgjidhni furnitorin'), true);
text_row(_('Nr. fatures se furnitorit:'), 'supplier_invoice_no', null, 28, 60);
date_row(_('Data e fatures:'), 'supplier_invoice_date');
array_selector_row(_('Fatura ne FrontAccounting:'), 'supplier_trans_no', null,
    ks_import_supplier_invoices((int)get_post('supplier_id')));
text_row(_('Shteti i origjines:'), 'origin_country', null, 28, 80);
text_row(_('Shteti i dergimit:'), 'dispatch_country', null, 28, 80);
text_row(_('Incoterm:'), 'incoterm', null, 12, 20);

table_section_title(_('DUD dhe dogana'));
text_row(_('Numri DUD:'), 'dud_no', null, 28, 80);
date_row(_('Data DUD:'), 'dud_date');
text_row(_('Zyra doganore:'), 'customs_office', null, 28, 120);

table_section(2);
table_section_title(_('Vlera e fatures'));
currencies_list_row(_('Monedha:'), 'currency');
amount_row(_('Kursi ne EUR:'), 'exchange_rate', null, null, null, 6);
amount_row(_('Vlera e mallit ne monedhe:'), 'goods_amount_currency');
label_row(_('Vlera e mallit ne EUR:'), price_format($row['goods_amount_base']));

table_section_title(_('Transporti dhe kostot percjellese'));
array_selector_row(_('Trajtimi i transportit:'), 'transport_mode', null, array(
    'included' => _('I perfshire ne faturen e mallit'),
    'separate' => _('Fature e vecante'),
    'none' => _('Pa transport')
));
text_row(_('Nr. fatures se transportit:'), 'transport_invoice_no', null, 28, 60);
supplier_list_row(_('Transportuesi:'), 'transport_supplier_id', null, _('Pa transportues te lidhur'), true);
text_row(_('Nr. transaksionit FA te transportit:'), 'transport_trans_no', null, 12, 12);
amount_row(_('Transporti EUR:'), 'transport_amount_base');
amount_row(_('Sigurimi EUR:'), 'insurance_amount_base');
amount_row(_('Ngarkim/manipulim EUR:'), 'handling_amount_base');
amount_row(_('Vlera doganore sipas DUD (opsionale):'), 'customs_value_override');
amount_row(_('Detyrimi doganor EUR:'), 'customs_duty_base');
amount_row(_('Akciza EUR:'), 'excise_base');
amount_row(_('Terminali EUR:'), 'terminal_amount_base');
amount_row(_('Shpedicioni EUR:'), 'forwarding_amount_base');
amount_row(_('Kosto tjera EUR:'), 'other_amount_base');

table_section_title(_('TVSH dhe shperndarja'));
amount_row(_('Norma TVSH ne import:'), 'import_vat_rate', null, null, '%', 2);
amount_row(_('Baza e TVSH-se sipas DUD (opsionale):'), 'import_vat_base_override');
amount_row(_('TVSH sipas DUD (opsionale):'), 'import_vat_amount_override');
array_selector_row(_('Metoda e shperndarjes:'), 'allocation_method', null, array(
    'value' => _('Sipas vleres'), 'quantity' => _('Sipas sasise'),
    'weight' => _('Sipas peshes'), 'manual' => _('Manuale')
));
textarea_row(_('Shenime:'), 'notes', null, 36, 4, 1000);
end_outer_table(1);

$preview = ks_import_calculate(ks_import_post_data());
start_table(TABLESTYLE, "width='70%'");
table_header(array(_('Kontrolli'), _('EUR')));
label_row(_('Vlera doganore'), price_format($preview['customs_value_base']));
label_row(_('Baza e TVSH-se ne import'), price_format($preview['import_vat_base']));
label_row(_('TVSH ne import'), price_format($preview['import_vat_amount']));
label_row(_('Landed cost pa TVSH te zbritshme'), price_format($preview['landed_total']));
end_table(1);

if ($row['status'] == 'draft') {
    start_table(TABLESTYLE_NOBORDER);
    start_row();
    submit_cells('save', _('Ruaj draftin'), '', '', 'default');
    if ($import_id) {
        submit_cells('load_fa_invoice', _('Merr faturen dhe artikujt nga FA'), '',
            _('Ngarkon vetem kur dosja nuk ka artikuj.'), true);
    }
    end_row();
    end_table();
}
end_form();

if ($import_id) {
    display_heading(_('Artikujt dhe landed cost'));
    $items = ks_import_items($import_id);
    start_form();
    hidden('import_id', $import_id);
    start_table(TABLESTYLE, "width='95%'");
    table_header(array(_('Kodi'), _('Emertimi'), _('Sasia'), _('Pesha'), _('Vlera EUR'),
        _('Kosto shtese'), _('Landed cost'), _('Per njesi'), ''));
    $k = 0;
    foreach ($items as $item) {
        $extra = $item['duty_allocated'] + $item['excise_allocated'] + $item['transport_allocated'] + $item['insurance_allocated'] +
            $item['handling_allocated'] + $item['terminal_allocated'] + $item['forwarding_allocated'] +
            $item['other_allocated'];
        alt_table_row_color($k);
        label_cell($item['stock_id']);
        label_cell($item['description']);
        qty_cell($item['quantity']);
        qty_cell($item['weight']);
        amount_cell($item['invoice_value_base']);
        amount_cell($extra);
        amount_cell($item['landed_cost']);
        amount_cell($item['unit_landed_cost']);
        if ($row['status'] == 'draft') {
            delete_button_cell('DeleteItem'.$item['id'], _('Fshi'));
        } else {
            label_cell('');
        }
        end_row();
    }
    if ($row['status'] == 'draft') {
        start_row();
        stock_items_list_cells(null, 'new_stock_id', null, _('Pa kod / artikull i lire'));
        text_cells(null, 'new_description', null, 32, 255);
        qty_cells(null, 'new_quantity', null);
        qty_cells(null, 'new_weight', null);
        amount_cells(null, 'new_invoice_value', null);
        qty_cells(null, 'new_manual_share', null);
        label_cell(_('Llogaritet pas ruajtjes'));
        label_cell('');
        submit_cells('add_item', _('Shto'), '', _('Shto artikullin'), true);
        end_row();
    }
    end_table(1);
    if ($row['status'] == 'draft') {
        submit_center('mark_ready', _('Kontrollo dhe sheno Gati'), true,
            _('Ky veprim nuk poston asnje transaksion financiar.'), 'default');
    } elseif ($row['status'] == 'ready') {
        submit_center('reopen', _('Rikthe ne Draft'), true, '', 'default');
    }
    end_form();
}

end_page();
