<?php
$page_security = 'SA_KS_IMPORT_ENTRY';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();

if (isset($_GET['New'])) {
    unset($_SESSION['KS_IMPORT_PURCHASE']);
}

$_SESSION['KS_IMPORT_PURCHASE'] = array(
    'active' => true,
    'data' => array()
);

meta_forward($path_to_root.'/purchasing/po_entry_items.php',
    'NewInvoice=Yes&ImportPurchase=Yes');
