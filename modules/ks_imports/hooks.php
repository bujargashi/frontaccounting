<?php

define('SS_KS_IMPORTS', 110 << 8);

class hooks_ks_imports extends hooks
{
    var $module_name = 'ks_imports';

    function install_options($app)
    {
        global $path_to_root;

        if ($app->id == 'AP') {
            $app->add_lapp_function(0, _('Blerje nga Importi'),
                $path_to_root.'/modules/'.$this->module_name.'/purchase_import.php?New=1',
                'SA_KS_IMPORT_ENTRY', MENU_TRANSACTION);
            $app->add_lapp_function(1, _('Regjistri i Blerjeve nga Importi'),
                $path_to_root.'/modules/'.$this->module_name.'/imports.php',
                'SA_KS_IMPORT_VIEW', MENU_INQUIRY);
        }
    }

    function install_access()
    {
        $security_sections[SS_KS_IMPORTS] = _('Blerje nga Importi');
        $security_areas['SA_KS_IMPORT_ENTRY'] = array(SS_KS_IMPORTS | 1,
            _('Regjistrimi i blerjeve nga importi'));
        $security_areas['SA_KS_IMPORT_VIEW'] = array(SS_KS_IMPORTS | 2,
            _('Shikimi dhe raportimi i blerjeve nga importi'));

        return array($security_areas, $security_sections);
    }

    function activate_extension($company, $check_only = true)
    {
        $updates = array('install_4.0.sql' => array('ks_import_documents'));
        return $this->update_databases($company, $updates, $check_only);
    }
}
