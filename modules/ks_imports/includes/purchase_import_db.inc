<?php

function kspi_user() { return isset($_SESSION['wa_current_user']->username) ? $_SESSION['wa_current_user']->username : ''; }
function kspi_date_sql($d) { return $d ? db_escape(date2sql($d)) : 'NULL'; }
function kspi_setting($key,$default='') {
    $r=db_query("SELECT setting_value FROM ".TB_PREF."ks_purchase_import_settings WHERE setting_key=".db_escape($key));
    $v=db_fetch($r); return $v ? $v[0] : $default;
}
function kspi_set_setting($key,$value) {
    db_query("INSERT INTO ".TB_PREF."ks_purchase_import_settings(setting_key,setting_value) VALUES(".
        db_escape($key).",".db_escape($value).") ON DUPLICATE KEY UPDATE setting_value=VALUES(setting_value)");
}
function kspi_next_ref() {
    $p='IMP-'.date('Y').'-';
    $r=db_query("SELECT MAX(CAST(SUBSTRING(reference,".(strlen($p)+1).") AS UNSIGNED)) FROM ".TB_PREF.
        "ks_purchase_imports WHERE reference LIKE ".db_escape($p.'%'));
    $v=db_fetch($r); return $p.str_pad((int)$v[0]+1,4,'0',STR_PAD_LEFT);
}
function kspi_get($id) {
    $r=db_query("SELECT h.*,s.supp_name FROM ".TB_PREF."ks_purchase_imports h LEFT JOIN ".TB_PREF.
        "suppliers s ON s.supplier_id=h.supplier_id WHERE h.id=".(int)$id);
    return db_fetch_assoc($r);
}
function kspi_lines($id) {
    $out=array(); $r=db_query("SELECT * FROM ".TB_PREF."ks_purchase_import_lines WHERE import_id=".(int)$id." ORDER BY id");
    while($v=db_fetch_assoc($r)) $out[]=$v; return $out;
}
function kspi_save_header($id,$d) {
    $fields=array(
      'reference'=>db_escape($d['reference']),'supplier_id'=>(int)$d['supplier_id'],
      'supplier_invoice_no'=>db_escape($d['supplier_invoice_no']),'invoice_date'=>kspi_date_sql($d['invoice_date']),
      'due_date'=>kspi_date_sql($d['due_date']),'currency'=>db_escape($d['currency']),
      'exchange_rate'=>(float)$d['exchange_rate'],'location_code'=>db_escape($d['location_code']),
      'origin_country'=>db_escape($d['origin_country']),'dispatch_country'=>db_escape($d['dispatch_country']),
      'dud_no'=>db_escape($d['dud_no']),'dud_date'=>kspi_date_sql($d['dud_date']),
      'declaration_type'=>db_escape($d['declaration_type']),'customs_office'=>db_escape($d['customs_office']),
      'incoterm'=>db_escape($d['incoterm']),'transport_mode'=>db_escape($d['transport_mode']),
      'transport_supplier_id'=>(int)$d['transport_supplier_id'],'transport_trans_no'=>(int)$d['transport_trans_no'],
      'transport_invoice_no'=>db_escape($d['transport_invoice_no']),'freight_base'=>(float)$d['freight_base'],
      'insurance_base'=>(float)$d['insurance_base'],'handling_base'=>(float)$d['handling_base'],
      'terminal_base'=>(float)$d['terminal_base'],'forwarding_base'=>(float)$d['forwarding_base'],
      'other_base'=>(float)$d['other_base'],'allocation_method'=>db_escape($d['allocation_method']),
      'notes'=>db_escape($d['notes']),'updated_by'=>db_escape(kspi_user()),'updated_at'=>db_escape(date('Y-m-d H:i:s')));
    if($id) {
      $set=array(); foreach($fields as $k=>$v)$set[]="$k=$v";
      db_query("UPDATE ".TB_PREF."ks_purchase_imports SET ".implode(',',$set)." WHERE id=".(int)$id." AND status='draft'");
      return (int)$id;
    }
    $fields['created_by']=db_escape(kspi_user()); $fields['created_at']=db_escape(date('Y-m-d H:i:s'));
    db_query("INSERT INTO ".TB_PREF."ks_purchase_imports(".implode(',',array_keys($fields)).") VALUES(".implode(',',array_values($fields)).")");
    return db_insert_id();
}
function kspi_add_line($id,$d) {
    $item=get_item($d['stock_id']);
    db_query("INSERT INTO ".TB_PREF."ks_purchase_import_lines(import_id,stock_id,description,quantity,uom,
      unit_price_currency,line_currency,line_base,tariff_code,origin_country,net_weight,gross_weight,duty_rate,
      duty_override,excise_amount,vat_rate,vat_base_override,vat_amount_override) VALUES(".(int)$id.",".
      db_escape($d['stock_id']).",".db_escape($d['description'] ? $d['description'] : $item['description']).",".
      (float)$d['quantity'].",".db_escape($item['units']).",".(float)$d['unit_price_currency'].",0,0,".
      db_escape($d['tariff_code']).",".db_escape($d['origin_country']).",".(float)$d['net_weight'].",".
      (float)$d['gross_weight'].",".(float)$d['duty_rate'].",".(float)$d['duty_override'].",".
      (float)$d['excise_amount'].",".(float)$d['vat_rate'].",".(float)$d['vat_base_override'].",".
      (float)$d['vat_amount_override'].")");
}
function kspi_delete_line($id,$line) {
    db_query("DELETE l FROM ".TB_PREF."ks_purchase_import_lines l INNER JOIN ".TB_PREF.
      "ks_purchase_imports h ON h.id=l.import_id WHERE l.id=".(int)$line." AND l.import_id=".(int)$id." AND h.status='draft'");
}
function kspi_store_calculation($id,$lines,$totals) {
    foreach($lines as $l) db_query("UPDATE ".TB_PREF."ks_purchase_import_lines SET line_currency=".(float)$l['line_currency'].
      ",line_base=".(float)$l['line_base'].",customs_value=".(float)$l['customs_value'].",duty_amount=".(float)$l['duty_amount'].
      ",vat_base=".(float)$l['vat_base'].",vat_amount=".(float)$l['vat_amount'].",import_cost_allocated=".
      (float)$l['import_cost_allocated'].",landed_cost=".(float)$l['landed_cost'].",unit_landed_cost=".
      (float)$l['unit_landed_cost']." WHERE id=".(int)$l['id']." AND import_id=".(int)$id);
    $set=array(); foreach($totals as $k=>$v)$set[]="$k=".(float)$v;
    db_query("UPDATE ".TB_PREF."ks_purchase_imports SET ".implode(',',$set)." WHERE id=".(int)$id);
}
function kspi_transport_invoices($supplier) {
    $out=array(0=>_('Zgjidhni faturen')); if(!$supplier)return $out;
    $r=db_query("SELECT trans_no,supp_reference,tran_date,ov_amount,ov_gst FROM ".TB_PREF."supp_trans WHERE type=".
      ST_SUPPINVOICE." AND supplier_id=".(int)$supplier." ORDER BY tran_date DESC,trans_no DESC LIMIT 200");
    while($v=db_fetch_assoc($r))$out[$v['trans_no']]=sql2date($v['tran_date']).' | '.$v['supp_reference'].' | '.price_format($v['ov_amount']+$v['ov_gst']);
    return $out;
}
function kspi_mark_ready($id){db_query("UPDATE ".TB_PREF."ks_purchase_imports SET status='ready' WHERE id=".(int)$id." AND status='draft'");}
function kspi_reopen($id){db_query("UPDATE ".TB_PREF."ks_purchase_imports SET status='draft' WHERE id=".(int)$id." AND status='ready'");}
function kspi_mark_posted($id,$inv,$created,$po,$grn,$journal){db_query("UPDATE ".TB_PREF."ks_purchase_imports SET status='posted',supplier_trans_no=".(int)$inv.",supplier_invoice_created=".(int)$created.",po_no=".(int)$po.",grn_no=".(int)$grn.",journal_no=".(int)$journal.",posted_by=".db_escape(kspi_user()).",posted_at=".db_escape(date('Y-m-d H:i:s'))." WHERE id=".(int)$id." AND status='ready'");}
