<?php

include_once($path_to_root.'/modules/ks_imports/includes/native_import_db.inc');

function ks_import_defaults()
{
    return array(
        'dud_no' => '',
        'dud_date' => Today(),
        'clearance_date' => Today(),
        'customs_office' => '',
        'origin_country' => '',
        'incoterm' => '',
        'customs_exchange_rate' => 1,
        'tariff_codes' => '',
        'customs_value' => 0,
        'customs_duty' => 0,
        'excise' => 0,
        'import_vat_base' => 0,
        'import_vat' => 0,
        'eur1_reference' => '',
        'import_notes' => ''
    );
}

function ks_import_prepare_purchase_entry()
{
    if (isset($_GET['ImportPurchase'])) {
        $_SESSION['KS_IMPORT_PURCHASE']['active'] = true;
        if (!isset($_SESSION['KS_IMPORT_PURCHASE']['data'])) {
            $_SESSION['KS_IMPORT_PURCHASE']['data'] = ks_import_defaults();
        }
    } elseif (isset($_GET['NewOrder']) || isset($_GET['NewGRN']) ||
        isset($_GET['NewInvoice'])) {
        unset($_SESSION['KS_IMPORT_PURCHASE']);
    }

    if (ks_import_is_active() && count($_POST)) {
        $_SESSION['KS_IMPORT_PURCHASE']['data'] = array_merge(
            ks_import_defaults(), ks_import_post_data());
    }
}

function ks_import_is_active()
{
    return !empty($_SESSION['KS_IMPORT_PURCHASE']['active']);
}

function ks_import_fill_post()
{
    $data = isset($_SESSION['KS_IMPORT_PURCHASE']['data'])
        ? $_SESSION['KS_IMPORT_PURCHASE']['data'] : ks_import_defaults();
    $map = array(
        'dud_no' => 'ks_dud_no',
        'dud_date' => 'ks_dud_date',
        'clearance_date' => 'ks_clearance_date',
        'customs_office' => 'ks_customs_office',
        'origin_country' => 'ks_origin_country',
        'incoterm' => 'ks_incoterm',
        'customs_exchange_rate' => 'ks_customs_exchange_rate',
        'tariff_codes' => 'ks_tariff_codes',
        'customs_value' => 'ks_customs_value',
        'customs_duty' => 'ks_customs_duty',
        'excise' => 'ks_excise',
        'import_vat_base' => 'ks_import_vat_base',
        'import_vat' => 'ks_import_vat',
        'eur1_reference' => 'ks_eur1_reference',
        'import_notes' => 'ks_import_notes'
    );
    foreach ($map as $key => $post_name) {
        if (!isset($_POST[$post_name])) {
            $_POST[$post_name] = $data[$key];
        }
    }
}

function ks_import_display_fields()
{
    if (!ks_import_is_active()) {
        return;
    }
    ks_import_fill_post();
    display_heading(_('Te dhenat e importit dhe DUD-it'));
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
    display_note(_('Dogana, akciza dhe TVSH-ja regjistrohen ne llogarite perkatese me funksionet standarde te Blerjeve; keto fusha ruajne evidencen ligjore te DUD-it.'), 0, 1);
}

function ks_import_validate_purchase()
{
    if (!ks_import_is_active()) {
        return true;
    }
    $data = ks_import_post_data();
    $_SESSION['KS_IMPORT_PURCHASE']['data'] = array_merge(ks_import_defaults(), $data);
    return ks_import_validate_data($data);
}

function ks_import_store_native_invoice($trans_no, $cart)
{
    if (!ks_import_is_active() || $cart->trans_type != ST_SUPPINVOICE) {
        return true;
    }
    $data = ks_import_post_data();
    if (!ks_import_save_document($trans_no, $cart->supplier_id, $data)) {
        return false;
    }
    return ks_import_document_exists($trans_no, $cart->supplier_id);
}

function ks_import_clear_purchase_entry()
{
    unset($_SESSION['KS_IMPORT_PURCHASE']);
}
