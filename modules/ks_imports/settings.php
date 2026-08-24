<?php
$page_security='SA_KS_IMPORT_SETUP';$path_to_root='../..';
include_once($path_to_root.'/includes/session.inc');add_access_extensions();include_once($path_to_root.'/includes/ui.inc');
include_once($path_to_root.'/modules/ks_imports/includes/purchase_import_db.inc');
page(_($help_context='Cilesimet - Blerje nga importi'));
$keys=array('default_location'=>'','customs_payable_account'=>'','landed_cost_clearing_account'=>'','vat_tax_type_18'=>'0','vat_tax_type_8'=>'0');
if(isset($_POST['save_settings'])){begin_transaction();foreach($keys as $k=>$d)kspi_set_setting($k,get_post($k,$d));commit_transaction();display_notification(_('Cilesimet u ruajten.'));}
foreach($keys as $k=>$d)if(!isset($_POST[$k]))$_POST[$k]=kspi_setting($k,$d);
display_heading(_('CILESIMET - BLERJE NGA IMPORTI'));
display_note(_('Keto llogari perdoren ne postimin native te stokut, Doganes, TVSH-se dhe landed cost.'),0,1);
start_form();start_table(TABLESTYLE2);locations_list_row(_('Depoja standarde:'),'default_location');gl_all_accounts_list_row(_('Detyrimi ndaj Doganes:'),'customs_payable_account');gl_all_accounts_list_row(_('Llogaria kalimtare e landed cost:'),'landed_cost_clearing_account');tax_types_list_row(_('Lloji tatimor - TVSH 18%:'),'vat_tax_type_18',null,_('Zgjidhni'));tax_types_list_row(_('Lloji tatimor - TVSH 8%:'),'vat_tax_type_8',null,_('Zgjidhni'));end_table(1);submit_center('save_settings',_('Ruaj cilesimet'),true,'','default');end_form();end_page();
