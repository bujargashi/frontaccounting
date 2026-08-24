<?php

global $reports;

$reports->addReport(RC_SUPPLIER, '_ks_import_register',
    _('Regjistri i Blerjeve nga Importi'),
    array(
        _('Nga data') => 'DATEBEGINM',
        _('Deri me daten') => 'DATEENDM',
        _('Furnitori') => 'SUPPLIERS_NO_FILTER',
        _('Komente') => 'TEXTBOX',
        _('Destinacioni') => 'DESTINATION'
    ));
