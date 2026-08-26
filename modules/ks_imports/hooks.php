<?php

class hooks_ks_imports extends hooks
{
    function activate_extension($company, $check_only = true)
    {
        $updates = array('install_4.0.sql' => array('ks_import_documents'));
        return $this->update_databases($company, $updates, $check_only);
    }
}
