# Kosovo Imports for FrontAccounting 2.4.18

Moduli regjistron dosjen e importit, DUD-in, faturen e furnitorit, transportin,
sigurimin, doganen, akcizen, terminalin, shpedicionin dhe kostot tjera. Ai llogarit:

- vleren doganore pa numeruar transportin dy here;
- bazen dhe vleren e TVSH-se ne import;
- landed cost sipas vleres, sasise, peshes ose perqindjes manuale;
- regjistrin CSV per kontroll dhe integrim me librin e blerjeve.

Versioni 2.4.0-1 nuk poston automatikisht transaksione ne Librin Kryesor. Dosja kalon
nga `Draft` ne `Gati` pas kontrollit. Postimi financiar duhet te aktivizohet vetem pasi
llogarite kontabel dhe formati i modulit ATK te jene verifikuar ne instalimin real.

