<?php

$page_security = 'SA_KS_IMPORT_VIEW';
$path_to_root = '../..';

include_once($path_to_root.'/includes/session.inc');
add_access_extensions();
include_once($path_to_root.'/includes/date_functions.inc');

$from = isset($_GET['from']) ? $_GET['from'] : begin_month();
$to = isset($_GET['to']) ? $_GET['to'] : Today();

$sql = "SELECT i.reference, i.dud_no, i.dud_date, i.customs_office,
        i.origin_country, i.supplier_invoice_no, i.supplier_invoice_date,
        s.supp_name, s.gst_no, i.customs_value_base, i.customs_duty_base,
        i.import_vat_base, i.import_vat_rate, i.import_vat_amount,
        i.transport_amount_base, i.insurance_amount_base,
        i.terminal_amount_base, i.forwarding_amount_base, i.other_amount_base,
        i.status
    FROM ".TB_PREF."ks_imports i
    LEFT JOIN ".TB_PREF."suppliers s ON s.supplier_id=i.supplier_id
    WHERE i.status IN ('ready','posted')
      AND i.dud_date >= ".db_escape(date2sql($from))."
      AND i.dud_date <= ".db_escape(date2sql($to))."
    ORDER BY i.dud_date, i.id";
$result = db_query($sql, _('Eksporti nuk mund te pergatitet.'));

$filename = 'KS_Importet_'.date2sql($from).'_'.date2sql($to).'.csv';
header('Content-Type: text/csv; charset=UTF-8');
header('Content-Disposition: attachment; filename="'.$filename.'"');
echo "\xEF\xBB\xBF";
$out = fopen('php://output', 'w');
fputcsv($out, array(
    'Reference', 'DUD Number', 'DUD Date', 'Customs Office', 'Origin Country',
    'Supplier Invoice', 'Supplier Invoice Date', 'Supplier', 'Supplier Fiscal No',
    'Customs Value EUR', 'Customs Duty EUR', 'Import VAT Base EUR',
    'Import VAT Rate', 'Import VAT EUR', 'Transport EUR', 'Insurance EUR',
    'Terminal EUR', 'Forwarding EUR', 'Other Costs EUR', 'Status'
));
while ($row = db_fetch_assoc($result)) {
    fputcsv($out, array(
        $row['reference'], $row['dud_no'], $row['dud_date'], $row['customs_office'],
        $row['origin_country'], $row['supplier_invoice_no'], $row['supplier_invoice_date'],
        $row['supp_name'], $row['gst_no'], $row['customs_value_base'],
        $row['customs_duty_base'], $row['import_vat_base'], $row['import_vat_rate'],
        $row['import_vat_amount'], $row['transport_amount_base'],
        $row['insurance_amount_base'], $row['terminal_amount_base'],
        $row['forwarding_amount_base'], $row['other_amount_base'], $row['status']
    ));
}
fclose($out);
exit;
