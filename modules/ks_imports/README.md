# Blerje nga Importi

Ky modul e hap dhe e poston blerjen me rrjedhen standarde **Direct Supplier

## Ndarja e detyrimeve te importit

Vetem per Blerjet nga Importi ruhet nje draft automatik:

- furnitori i mallit: vlera neto e fatures, pa TVSH;
- shpediteri: dogana, akciza, TVSH-ja e importit dhe shpenzimet tjera doganore;
- transportuesi: transporti, gjithmone jashte detyrimit te shpediterit.

Drafti nuk krijon pagese bankare dhe nuk prek Blerjet vendore. Funksioni mund te
caktivizohet me `KS_IMPORT_PAYMENT_SPLIT_ENABLED=false` ose duke kthyer commit-in;
tabela e draftit mbetet e izoluar dhe faturat native te FrontAccounting nuk preken.
Invoice** te FrontAccounting 2.4.18. Artikujt, sasite, cmimet, taksat, stoku,
Supplier Invoice, GL, pagesat, kredit-notat, alokimet dhe attachment-et vazhdojne
te trajtohen nga funksionet native te Purchases.

Moduli shton vetem evidencen e kerkuar per import: DUD-in, zhdoganimin,
origjinen, Incoterm-in, kursin dhe vlerat doganore, doganen, akcizen, bazen dhe
TVSH-ne e importit, EUR.1 dhe kodet tarifore. Te dhenat lidhen me numrin native
te Supplier Invoice dhe mund te perditesohen e raportohen pa krijuar nje sistem
paralel kontabel.
