<?php

class hooks_ks_imports extends hooks
{
    var $module_name = 'ks_imports';

    function install_options($app)
    {
        global $path_to_root;

        if ($app->id == 'AP') {
            $app->add_lapp_function(0, _('Blerje nga Importi'),
                $path_to_root.'/modules/'.$this->module_name.'/purchase_import.php?New=1',
                'SA_SUPPLIERINVOICE', MENU_TRANSACTION);
            $app->add_lapp_function(1, _('Regjistri i Blerjeve nga Importi'),
                $path_to_root.'/modules/'.$this->module_name.'/imports.php',
                'SA_SUPPTRANSVIEW', MENU_INQUIRY);
        }
    }

    function activate_extension($company, $check_only = true)
    {
        $updates = array('install_4.0.sql' => array('ks_import_documents'));
        return $this->update_databases($company, $updates, $check_only);
    }
}
