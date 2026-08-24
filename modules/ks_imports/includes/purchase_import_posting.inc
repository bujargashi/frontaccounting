<?php
include_once($path_to_root.'/purchasing/includes/purchasing_db.inc');
include_once($path_to_root.'/purchasing/includes/po_class.inc');
include_once($path_to_root.'/gl/includes/db/gl_journal.inc');
include_once($path_to_root.'/gl/includes/gl_db.inc');

function kspi_tax_type_for_rate($rate) {
    $key='vat_tax_type_'.str_replace('.','_',rtrim(rtrim(number_format($rate,4,'.',''),'0'),'.'));
    $id=(int)kspi_setting($key,0); if($id)return get_tax_type($id);
    $r=db_query("SELECT * FROM ".TB_PREF."tax_types WHERE inactive=0 AND ABS(rate-".(float)$rate.")<0.0001 LIMIT 1");
    return db_fetch_assoc($r);
}
function kspi_validate_post($h,$lines) {
    global $Refs; $e=array();
    if(!$h||$h['status']!='ready')$e[]=_('Vetem dokumenti i kontrolluar mund te postohet.');
    if(!count($lines))$e[]=_('Nuk ka artikuj.');
    foreach(array('supplier_id','supplier_invoice_no','invoice_date','location_code','dud_no','dud_date') as $f)
      if(empty($h[$f]))$e[]=_('Mungon fusha e detyrueshme: ').$f;
    if(is_reference_already_there($h['supplier_id'],$h['supplier_invoice_no']))$e[]=_('Numri i fatures ekziston per kete furnitor.');
    $supplier=get_supplier($h['supplier_id']);
    foreach($lines as $l) {
      $item=get_item($l['stock_id']); if(!$item||$item['mb_flag']!='B')$e[]=sprintf(_('%s nuk eshte artikull inventari i blershem.'),$l['stock_id']);
      if((float)$l['vat_amount'] && !kspi_tax_type_for_rate((float)$l['vat_rate']))$e[]=sprintf(_('Nuk ka lloj tatimor per normen %s%%.'),$l['vat_rate']);
    }
    if(((float)$h['duty_base']+(float)$h['excise_base']+(float)$h['vat_amount'])&&!kspi_setting('customs_payable_account'))$e[]=_('Mungon llogaria e Doganes.');
    $ops=($h['transport_mode']=='separate'?(float)$h['freight_base']:0)+(float)$h['insurance_base']+(float)$h['handling_base']+(float)$h['terminal_base']+(float)$h['forwarding_base']+(float)$h['other_base'];
    if($ops&&!kspi_setting('landed_cost_clearing_account'))$e[]=_('Mungon llogaria kalimtare e landed cost.');
    if($h['transport_mode']=='separate'&&(float)$h['freight_base']>0&&!(int)$h['transport_trans_no'])$e[]=_('Lidhni faturen e transportit te regjistruar ne Blerje.');
    return $e;
}
function kspi_create_purchase_invoice($h,$lines) {
    global $Refs; $s=get_supplier($h['supplier_id']); $c=new purch_order; $c->trans_type=ST_SUPPINVOICE;
    $c->set_supplier($h['supplier_id'],$s['supp_name'],$s['curr_code'],$s['tax_group_id'],$s['tax_included']);
    $c->orig_order_date=sql2date($h['invoice_date']); $c->due_date=sql2date($h['due_date']);
    $c->reference=$Refs->get_next(ST_SUPPINVOICE,null,array('date'=>$c->orig_order_date,'supplier'=>$h['supplier_id'],'location'=>$h['location_code']));
    $c->supp_ref=$h['supplier_invoice_no']; $c->Comments='IMPORT '.$h['reference'].'; DUD '.$h['dud_no'];
    $c->Location=$h['location_code']; $c->delivery_address=''; $c->dimension=$s['dimension_id']; $c->dimension2=$s['dimension2_id'];
    $c->cash_account=0; $c->ex_rate=(float)$h['exchange_rate']; $n=0;
    foreach($lines as $l)$c->add_to_order($n++,$l['stock_id'],(float)$l['quantity'],$l['description'],(float)$l['unit_price_currency'],$l['uom'],$c->orig_order_date,0,0);
    foreach($c->get_taxes() as $tax)if(abs((float)$tax['Value'])>.005)return array(0,_('Furnitori i huaj duhet te kete grup tatimor pa TVSH vendore.'));
    return array(add_direct_supp_trans($c),'');
}
function kspi_native_docs($inv){$r=db_query("SELECT MIN(p.order_no) po_no,MIN(g.grn_batch_id) grn_no FROM ".TB_PREF."supp_invoice_items i LEFT JOIN ".TB_PREF."grn_items g ON g.id=i.grn_item_id LEFT JOIN ".TB_PREF."purch_order_details p ON p.po_detail_item=i.po_detail_item_id WHERE i.supp_trans_type=".ST_SUPPINVOICE." AND i.supp_trans_no=".(int)$inv);return db_fetch_assoc($r);}
function kspi_post_journal($h,$lines) {
    global $Refs; $extra=round((float)$h['landed_total']-(float)$h['goods_base'],2); $vat=(float)$h['vat_amount'];
    if(!$extra&&!$vat)return 0; $date=sql2date($h['dud_date']); $no=get_next_trans_no(ST_JOURNAL); $ref=$Refs->get_next(ST_JOURNAL,null,$date); $memo='IMPORT '.$h['reference'].'; DUD '.$h['dud_no'];
    add_journal(ST_JOURNAL,$no,$extra+$vat,$date,get_company_currency(),$ref,$h['dud_no']);
    $vatgroups=array();
    foreach($lines as $l){$x=(float)$l['landed_cost']-(float)$l['line_base'];if($x){$g=get_stock_gl_code($l['stock_id']);add_gl_trans_std_cost(ST_JOURNAL,$no,$date,$g['inventory_account'],$g['dimension_id'],$g['dimension2_id'],$memo.' '.$l['stock_id'],$x);update_average_material_cost(null,$l['stock_id'],$x/(float)$l['quantity'],(float)$l['quantity'],$date,true);} $k=(string)(float)$l['vat_rate'];if(!isset($vatgroups[$k]))$vatgroups[$k]=array('base'=>0,'vat'=>0);$vatgroups[$k]['base']+=(float)$l['vat_base'];$vatgroups[$k]['vat']+=(float)$l['vat_amount'];}
    foreach($vatgroups as $rate=>$v)if(abs($v['vat'])>.005){$t=kspi_tax_type_for_rate((float)$rate);add_gl_trans_std_cost(ST_JOURNAL,$no,$date,$t['purchasing_gl_code'],0,0,$memo,$v['vat']);add_trans_tax_details(ST_JOURNAL,$no,$t['id'],(float)$rate,false,$v['vat'],$v['base'],1,$date,$h['dud_no'],TR_INPUT);}
    $customs=(float)$h['duty_base']+(float)$h['excise_base']+$vat;if($customs)add_gl_trans_std_cost(ST_JOURNAL,$no,$date,kspi_setting('customs_payable_account'),0,0,$memo,-$customs);
    $clearing=$extra-(float)$h['duty_base']-(float)$h['excise_base'];if($clearing)add_gl_trans_std_cost(ST_JOURNAL,$no,$date,kspi_setting('landed_cost_clearing_account'),0,0,$memo,-$clearing);
    add_comments(ST_JOURNAL,$no,$date,$memo);add_audit_trail(ST_JOURNAL,$no,$date);$Refs->save(ST_JOURNAL,$no,$ref);return $no;
}
function kspi_post($id){$h=kspi_get($id);$l=kspi_lines($id);$e=kspi_validate_post($h,$l);if(count($e))return array(false,$e);begin_transaction();list($inv,$m)=kspi_create_purchase_invoice($h,$l);if(!$inv){cancel_transaction();return array(false,array($m));}$d=kspi_native_docs($inv);$j=kspi_post_journal($h,$l);kspi_mark_posted($id,$inv,1,$d['po_no'],$d['grn_no'],$j);commit_transaction();return array(true,array());}
