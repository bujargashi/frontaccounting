<?php

function ks_import_num($value)
{
    if (is_string($value)) {
        $value = str_replace(array(' ', ','), array('', '.'), $value);
    }
    return round((float)$value, 2);
}

function ks_import_calculate($data)
{
    $goods_currency = ks_import_num(@$data['goods_amount_currency']);
    $exchange_rate = (float)@$data['exchange_rate'];
    if ($exchange_rate <= 0) {
        $exchange_rate = 1;
    }

    $goods_base = round($goods_currency * $exchange_rate, 2);
    $transport = ks_import_num(@$data['transport_amount_base']);
    $insurance = ks_import_num(@$data['insurance_amount_base']);
    $handling = ks_import_num(@$data['handling_amount_base']);
    $duty = ks_import_num(@$data['customs_duty_base']);
    $excise = ks_import_num(@$data['excise_base']);
    $terminal = ks_import_num(@$data['terminal_amount_base']);
    $forwarding = ks_import_num(@$data['forwarding_amount_base']);
    $other = ks_import_num(@$data['other_amount_base']);
    $transport_mode = @$data['transport_mode'];

    // Transport already included in the supplier invoice is part of goods_base
    // and must never be added for a second time.
    $customs_transport = $transport_mode == 'separate' ? $transport : 0;
    $customs_value_calculated = round($goods_base + $customs_transport + $insurance + $handling, 2);
    $customs_value_override = ks_import_num(@$data['customs_value_override']);
    $customs_value = $customs_value_override > 0 ? $customs_value_override : $customs_value_calculated;
    $vat_base_calculated = round($customs_value + $duty + $excise, 2);
    $vat_base_override = ks_import_num(@$data['import_vat_base_override']);
    $vat_base = $vat_base_override > 0 ? $vat_base_override : $vat_base_calculated;
    $vat_rate = (float)@$data['import_vat_rate'];
    if ($vat_rate < 0) {
        $vat_rate = 0;
    }
    $vat_amount_calculated = round($vat_base * $vat_rate / 100, 2);
    $vat_amount_override = ks_import_num(@$data['import_vat_amount_override']);
    $vat_amount = $vat_amount_override > 0 ? $vat_amount_override : $vat_amount_calculated;
    $landed_transport = $transport_mode == 'separate' ? $transport : 0;
    $landed_total = round($goods_base + $duty + $excise + $landed_transport + $insurance + $handling + $terminal + $forwarding + $other, 2);

    return array(
        'exchange_rate' => $exchange_rate,
        'goods_amount_base' => $goods_base,
        'customs_value_calculated' => $customs_value_calculated,
        'customs_value_base' => $customs_value,
        'import_vat_base_calculated' => $vat_base_calculated,
        'import_vat_base' => $vat_base,
        'import_vat_amount_calculated' => $vat_amount_calculated,
        'import_vat_amount' => $vat_amount,
        'landed_total' => $landed_total
    );
}

function ks_import_allocate($items, $header)
{
    $method = @$header['allocation_method'];
    $basis_total = 0;
    foreach ($items as $item) {
        if ($method == 'quantity') {
            $basis_total += (float)$item['quantity'];
        } elseif ($method == 'weight') {
            $basis_total += (float)$item['weight'];
        } elseif ($method == 'manual') {
            $basis_total += (float)$item['manual_share'];
        } else {
            $basis_total += (float)$item['invoice_value_base'];
        }
    }

    $pools = array(
        'duty_allocated' => (float)$header['customs_duty_base'],
        'excise_allocated' => (float)$header['excise_base'],
        'transport_allocated' => @$header['transport_mode'] == 'separate'
            ? (float)$header['transport_amount_base'] : 0,
        'insurance_allocated' => (float)$header['insurance_amount_base'],
        'handling_allocated' => (float)$header['handling_amount_base'],
        'terminal_allocated' => (float)$header['terminal_amount_base'],
        'forwarding_allocated' => (float)$header['forwarding_amount_base'],
        'other_allocated' => (float)$header['other_amount_base']
    );

    $allocated = array();
    $count = count($items);
    foreach ($items as $index => $item) {
        if ($method == 'quantity') {
            $basis = (float)$item['quantity'];
        } elseif ($method == 'weight') {
            $basis = (float)$item['weight'];
        } elseif ($method == 'manual') {
            $basis = (float)$item['manual_share'];
        } else {
            $basis = (float)$item['invoice_value_base'];
        }
        $share = $basis_total > 0 ? $basis / $basis_total : 0;
        foreach ($pools as $field => $pool) {
            $item[$field] = round($pool * $share, 2);
        }
        $extra = 0;
        foreach ($pools as $field => $pool) {
            $extra += $item[$field];
        }
        $item['landed_cost'] = round((float)$item['invoice_value_base'] + $extra, 2);
        $item['unit_landed_cost'] = (float)$item['quantity'] > 0
            ? round($item['landed_cost'] / (float)$item['quantity'], 6) : 0;
        $allocated[] = $item;
    }

    // Put rounding differences on the final line so the allocated totals always reconcile.
    if ($count) {
        $last = $count - 1;
        foreach ($pools as $field => $pool) {
            $sum = 0;
            foreach ($allocated as $row) {
                $sum += $row[$field];
            }
            $allocated[$last][$field] = round($allocated[$last][$field] + ($pool - $sum), 2);
        }
        $extra = 0;
        foreach ($pools as $field => $pool) {
            $extra += $allocated[$last][$field];
        }
        $allocated[$last]['landed_cost'] = round((float)$allocated[$last]['invoice_value_base'] + $extra, 2);
        $allocated[$last]['unit_landed_cost'] = (float)$allocated[$last]['quantity'] > 0
            ? round($allocated[$last]['landed_cost'] / (float)$allocated[$last]['quantity'], 6) : 0;
    }

    return $allocated;
}
