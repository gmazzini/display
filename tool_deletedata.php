<?php


include "data.php";
$conn=oci_connect($p1,$p2,$p3);

function mydelete($conn,$table){
  $query=oci_parse($conn,"delete from $table");
  oci_execute($query);
  oci_free_statement($query);
}

mydelete($conn,"idistat");
mydelete($conn,"istatente");
mydelete($conn,"man");
mydelete($conn,"pal");
mydelete($conn,"scuole");
mydelete($conn,"areeaai");
mydelete($conn,"uiftth");
mydelete($conn,"aziendeaai");
mydelete($conn,"apwifi");
mydelete($conn,"attivifse");
mydelete($conn,"accessifse");
mydelete($conn,"scaricatifse");
mydelete($conn,"attivazionilepidaid");
mydelete($conn,"accessilepidaid");
mydelete($conn,"sportellilepidaid");
mydelete($conn,"userwifi");

oci_close($conn);

?>
