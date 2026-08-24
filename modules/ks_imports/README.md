# Blerje nga Importi

Ky modul e hap dhe e poston blerjen me rrjedhen standarde **Direct Supplier
Invoice** te FrontAccounting 2.4.18. Artikujt, sasite, cmimet, taksat, stoku,
Supplier Invoice, GL, pagesat, kredit-notat, alokimet dhe attachment-et vazhdojne
te trajtohen nga funksionet native te Purchases.

Moduli shton evidencen e kerkuar per import: DUD-in, zhdoganimin, origjinen,
Incoterm-in, kursin dhe vlerat doganore, doganen, akcizen, bazen dhe TVSH-ne e
importit, EUR.1 dhe kodet tarifore.

## Ndarja e detyrimeve te importit

Vetem per Blerjet nga Importi krijohet fillimisht nje draft:

- furnitori i mallit: mbetja e fatures pas ndarjes, para avanseve;
- shpediteri: dogana, akciza, TVSH-ja e importit dhe shpenzimet tjera doganore;
- transportuesi: vetem transporti qe paguhet me fature te vecante.

Kur transporti eshte ne faturen e furnitorit te huaj, fusha e transportuesit dhe
shuma e transportit te vecante lihen bosh. Avanset nuk jane te detyrueshme; kur
ekzistojne, FrontAccounting i alokon me funksionin standard.

Drafti mund te perditesohet ose anulohet pa krijuar dokument kontabel. Vetem pas
konfirmimit te dyte krijohen dokumentet standarde:

1. kredit-note ndaj furnitorit te mallit per shumen qe ndahet;
2. Supplier Invoice ndaj shpediterit;
3. Supplier Invoice ndaj transportuesit, kur transporti paguhet vecmas.

Keto dokumente vetem riklasifikojne detyrimin. Stoku, kostoja, dogana dhe TVSH-ja
e regjistruar ne blerjen origjinale nuk postohen perseri. Pagesat kryhen me
`Payment to Supplier` dhe alokohen ndaj faturave perkatese.

Blerjet vendore nuk preken. Funksioni mund te caktivizohet me
`KS_IMPORT_PAYMENT_SPLIT_ENABLED=false`; dokumentet e konfirmuara mbeten pjese e
auditimit standard te FrontAccounting dhe anulohen vetem me `Void a Transaction`.
