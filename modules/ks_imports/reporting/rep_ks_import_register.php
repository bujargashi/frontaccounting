<?php
$page_security = 'SA_KS_IMPORT_VIEW';
$path_to_root = '..';

include_once($path_to_root.'/includes/session.inc');
include_once($path_to_root.'/includes/date_functions.inc');
include_once($path_to_root.'/modules/ks_imports/includes/native_import_db.inc');

print_ks_import_register();

function print_ks_import_register()
{
    global $path_to_root;

    $from = $_POST['PARAM_0'];
    $to = $_POST['PARAM_1'];
    $supplier = $_POST['PARAM_2'];
    $comments = $_POST['PARAM_3'];
    $destination = $_POST['PARAM_4'];

    if ($destination) {
        include_once($path_to_root.'/reporting/includes/excel_report.inc');
    } else {
        include_once($path_to_root.'/reporting/includes/pdf_report.inc');
    }

    if ($supplier == ALL_NUMERIC) {
        $supplier = 0;
    }

    $cols = array(0, 55, 145, 205, 260, 325, 390, 455, 520);
    $headers = array(_('Data'), _('Furnitori'), _('Fatura'), _('DUD'),
        _('Vlera dog.'), _('Dogana'), _('Baza TVSH'), _('TVSH'));
    $aligns = array('left', 'left', 'left', 'left', 'right', 'right', 'right', 'right');
    $params = array(
        0 => $comments,
        1 => array('text' => _('Periudha'), 'from' => $from, 'to' => $to)
    );

    $rep = new FrontReport(_('Regjistri i Blerjeve nga Importi'),
        'KsImportRegister', user_pagesize());
    $rep->Font();
    $rep->Info($params, $cols, $headers, $aligns);
    $rep->NewPage();

    $res = ks_import_query($from, $to, $supplier, '');
    $tot_customs = $tot_duty = $tot_base = $tot_vat = 0;
    while ($row = db_fetch_assoc($res)) {
        $rep->TextCol(0, 1, sql2date($row['clearance_date']));
        $rep->TextCol(1, 2, $row['supp_name']);
        $rep->TextCol(2, 3, $row['supp_reference']);
        $rep->TextCol(3, 4, $row['dud_no']);
        $rep->AmountCol(4, 5, $row['customs_value'], user_price_dec());
        $rep->AmountCol(5, 6, $row['customs_duty'] + $row['excise'], user_price_dec());
        $rep->AmountCol(6, 7, $row['import_vat_base'], user_price_dec());
        $rep->AmountCol(7, 8, $row['import_vat'], user_price_dec());
        $rep->NewLine();
        $tot_customs += $row['customs_value'];
        $tot_duty += $row['customs_duty'] + $row['excise'];
        $tot_base += $row['import_vat_base'];
        $tot_vat += $row['import_vat'];
    }

    $rep->Line($rep->row - 2);
    $rep->TextCol(0, 4, _('TOTALI'));
    $rep->AmountCol(4, 5, $tot_customs, user_price_dec());
    $rep->AmountCol(5, 6, $tot_duty, user_price_dec());
    $rep->AmountCol(6, 7, $tot_base, user_price_dec());
    $rep->AmountCol(7, 8, $tot_vat, user_price_dec());
    $rep->End();
}
