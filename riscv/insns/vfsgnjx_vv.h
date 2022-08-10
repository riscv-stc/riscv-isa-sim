// vfsgnx
require(P.VU.vstart == 0);

VI_VFP_VV_LOOP
({
  vd = fsgnj16bf(vs2.v, vs1.v, false, true);
},
{
  vd = fsgnj16(vs2.v, vs1.v, false, true);
},
{
  vd = fsgnj32(vs2.v, vs1.v, false, true);
},
{
  vd = fsgnj64(vs2.v, vs1.v, false, true);
})
