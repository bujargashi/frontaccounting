ALTER TABLE 0_ks_import_payment_splits
  ADD goods_credit_type smallint NOT NULL DEFAULT 0 AFTER status,
  ADD goods_credit_no int unsigned NOT NULL DEFAULT 0 AFTER goods_credit_type,
  ADD forwarder_invoice_type smallint NOT NULL DEFAULT 0 AFTER goods_credit_no,
  ADD forwarder_invoice_no int unsigned NOT NULL DEFAULT 0 AFTER forwarder_invoice_type,
  ADD transport_invoice_type smallint NOT NULL DEFAULT 0 AFTER forwarder_invoice_no,
  ADD transport_invoice_no int unsigned NOT NULL DEFAULT 0 AFTER transport_invoice_type,
  ADD confirmed_by varchar(60) NOT NULL DEFAULT '' AFTER transport_invoice_no,
  ADD confirmed_at datetime NULL AFTER confirmed_by,
  ADD cancelled_by varchar(60) NOT NULL DEFAULT '' AFTER confirmed_at,
  ADD cancelled_at datetime NULL AFTER cancelled_by;
