<?php

function kspi_num($value)
{
    if (is_string($value)) $value = str_replace(array(' ', ','), array('', '.'), $value);
    return round((float)$value, 6);
}

function kspi_recalculate($header, $lines)
{
    $rate = max((float)$header['exchange_rate'], 0.00000001);
    $basis_total = 0;
    foreach ($lines as $line) {
        if ($header['allocation_method'] == 'quantity') $basis_total += (float)$line['quantity'];
        elseif ($header['allocation_method'] == 'weight') $basis_total += (float)$line['net_weight'];
        else $basis_total += (float)$line['line_base'];
    }
    $customs_pool = ($header['transport_mode'] == 'separate' ? (float)$header['freight_base'] : 0)
        + (float)$header['insurance_base'] + (float)$header['handling_base'];
    $other_pool = (float)$header['terminal_base'] + (float)$header['forwarding_base']
        + (float)$header['other_base'];
    $out = array(); $totals = array('goods_currency'=>0,'goods_base'=>0,'customs_value_base'=>0,
        'duty_base'=>0,'excise_base'=>0,'vat_base'=>0,'vat_amount'=>0,'landed_total'=>0);
    foreach ($lines as $line) {
        $line['line_currency'] = round((float)$line['quantity']*(float)$line['unit_price_currency'], 2);
        $line['line_base'] = round($line['line_currency']*$rate, 2);
        if ($header['allocation_method'] == 'quantity') $basis=(float)$line['quantity'];
        elseif ($header['allocation_method'] == 'weight') $basis=(float)$line['net_weight'];
        else $basis=(float)$line['line_base'];
        $share = $basis_total > 0 ? $basis/$basis_total : 0;
        $customs_alloc = round($customs_pool*$share, 2);
        $other_alloc = round($other_pool*$share, 2);
        $line['customs_value'] = round($line['line_base']+$customs_alloc, 2);
        $line['duty_amount'] = (float)$line['duty_override'] > 0 ? round((float)$line['duty_override'],2)
            : round($line['customs_value']*(float)$line['duty_rate']/100,2);
        $line['vat_base'] = (float)$line['vat_base_override'] > 0 ? round((float)$line['vat_base_override'],2)
            : round($line['customs_value']+$line['duty_amount']+(float)$line['excise_amount'],2);
        $line['vat_amount'] = (float)$line['vat_amount_override'] > 0 ? round((float)$line['vat_amount_override'],2)
            : round($line['vat_base']*(float)$line['vat_rate']/100,2);
        $line['import_cost_allocated'] = round($customs_alloc+$other_alloc+$line['duty_amount']+(float)$line['excise_amount'],2);
        $line['landed_cost'] = round($line['line_base']+$line['import_cost_allocated'],2);
        $line['unit_landed_cost'] = (float)$line['quantity'] > 0 ? round($line['landed_cost']/(float)$line['quantity'],6) : 0;
        $totals['goods_currency'] += $line['line_currency']; $totals['goods_base'] += $line['line_base'];
        $totals['customs_value_base'] += $line['customs_value']; $totals['duty_base'] += $line['duty_amount'];
        $totals['excise_base'] += (float)$line['excise_amount']; $totals['vat_base'] += $line['vat_base'];
        $totals['vat_amount'] += $line['vat_amount']; $totals['landed_total'] += $line['landed_cost'];
        $out[]=$line;
    }
    foreach ($totals as $key=>$value) $totals[$key]=round($value,2);
    return array($out,$totals);
}
