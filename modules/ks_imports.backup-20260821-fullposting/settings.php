<?php

$page_security = 'SA_KS_IMPORT_SETUP';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/gl/includes/gl_db.inc');
include_once($path_to_root.'/modules/ks_imports/includes/db.inc');

page(_($help_context = 'Cilesimet e importeve - Kosove'));

$keys = array(
    'default_vat_rate' => '18',
    'inventory_account' => '1510',
    'freight_account' => '5100',
    'input_vat_account' => '2150',
    'customs_payable_account' => '2100',
    'landed_cost_clearing_account' => '1550'
);

if (isset($_POST['save_settings'])) {
    begin_transaction();
    foreach ($keys as $key => $default) {
        ks_import_set_setting($key, get_post($key, $default));
    }
    commit_transaction();
    display_notification(_('Cilesimet u ruajten.'));
}

foreach ($keys as $key => $default) {
    if (!isset($_POST[$key])) {
        $_POST[$key] = ks_import_setting($key, $default);
    }
}

display_warning(_('Keto llogari perdoren vetem pasi te aktivizohet postimi financiar. Moduli nuk poston automatikisht pa kontroll dhe konfirmim.'));
start_form();
start_table(TABLESTYLE2);
table_section_title(_('Parametrat tatimore'));
amount_row(_('Norma standarde e TVSH-se ne import:'), 'default_vat_rate', null, null, '%', 2);
table_section_title(_('Llogarite e Librit Kryesor'));
gl_all_accounts_list_row(_('Stoku / inventari:'), 'inventory_account', null);
gl_all_accounts_list_row(_('Shpenzimi i transportit:'), 'freight_account', null);
gl_all_accounts_list_row(_('TVSH e zbritshme ne import:'), 'input_vat_account', null);
gl_all_accounts_list_row(_('Detyrimi ndaj Doganes:'), 'customs_payable_account', null);
gl_all_accounts_list_row(_('Llogaria kalimtare e landed cost:'), 'landed_cost_clearing_account', null);
end_table(1);
submit_center('save_settings', _('Ruaj cilesimet'), true, '', 'default');
end_form();

end_page();
