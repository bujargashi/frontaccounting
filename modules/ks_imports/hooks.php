<?php

class hooks_ks_imports extends hooks
{
    function activate_extension($company, $check_only = true)
    {
        $updates = array(
            'install_4.0.sql' => array('ks_import_documents'),
            'install_4.1.sql' => array('ks_import_payment_splits'),
            'install_4.2.sql' => array('ks_import_payment_splits',
                'goods_credit_no')
        );
        return $this->update_databases($company, $updates, $check_only);
    }
}
