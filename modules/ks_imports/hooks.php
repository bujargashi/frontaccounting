<?php

define('SS_KS_IMPORTS', 110 << 8);

class hooks_ks_imports extends hooks
{
    var $module_name = 'ks_imports';

    function install_options($app)
    {
        global $path_to_root;

        if ($app->id == 'AP') {
            $app->add_lapp_function(0, _('Blerje nga importi (Kosove)'),
                $path_to_root.'/modules/'.$this->module_name.'/purchase_import.php',
                'SA_KS_IMPORT_ENTRY', MENU_TRANSACTION);
            $app->add_lapp_function(1, _('Regjistri i blerjeve nga importi'),
                $path_to_root.'/modules/'.$this->module_name.'/imports.php',
                'SA_KS_IMPORT_VIEW', MENU_INQUIRY);
            $app->add_rapp_function(2, _('Cilesimet e importeve'),
                $path_to_root.'/modules/'.$this->module_name.'/settings.php',
                'SA_KS_IMPORT_SETUP', MENU_MAINTENANCE);
        }
    }

    function install_access()
    {
        $security_sections[SS_KS_IMPORTS] = _('Importet e Kosoves');
        $security_areas['SA_KS_IMPORT_ENTRY'] = array(SS_KS_IMPORTS | 1, _('Regjistrimi dhe ndryshimi i importeve'));
        $security_areas['SA_KS_IMPORT_VIEW'] = array(SS_KS_IMPORTS | 2, _('Shikimi i regjistrit te importeve'));
        $security_areas['SA_KS_IMPORT_SETUP'] = array(SS_KS_IMPORTS | 3, _('Cilesimet e modulit te importeve'));

        return array($security_areas, $security_sections);
    }

    function activate_extension($company, $check_only = true)
    {
        $updates = array('install_3.0.sql' => array('ks_purchase_imports'));
        return $this->update_databases($company, $updates, $check_only);
    }
}
