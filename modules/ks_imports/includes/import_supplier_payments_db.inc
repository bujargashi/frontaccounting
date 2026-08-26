<?php

function ks_import_supplier_obligations_query($supplier_id = 0)
{
    $sql = "SELECT i.trans_no, i.supplier_id, i.dud_no, i.dud_date,
        i.clearance_date,
        s.supp_name, s.curr_code, t.reference, t.supp_reference,
        t.tran_date, t.due_date, t.ov_amount, t.ov_gst, t.ov_discount,
        t.alloc,
        (t.ov_amount + t.ov_discount) AS net_total,
        GREATEST(0, (t.ov_amount + t.ov_discount) - t.alloc) AS net_due
        FROM ".TB_PREF."ks_import_documents i
        INNER JOIN ".TB_PREF."supp_trans t
            ON t.type=i.trans_type AND t.trans_no=i.trans_no
            AND t.supplier_id=i.supplier_id
        INNER JOIN ".TB_PREF."suppliers s
            ON s.supplier_id=i.supplier_id
        WHERE i.trans_type=".ST_SUPPINVOICE."
            AND (t.ov_amount + t.ov_discount) - t.alloc > 0.005";

    if ($supplier_id) {
        $sql .= " AND i.supplier_id=".(int)$supplier_id;
    }

    $sql .= " ORDER BY t.due_date, t.tran_date, i.trans_no";
    return db_query($sql,
        _('Detyrimet e furnitoreve te importit nuk mund te lexohen.'));
}
