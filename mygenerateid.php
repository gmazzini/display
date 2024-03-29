<?php

include "/home/admgm02/data.php";
include "/home/www/restdati.lepida.it/googleset.php";
$conn=oci_connect($p1,$p2,$p3);

echo "idistat\n";
$access_token=file_get_contents("/home/www/data/access_token");
$ch=curl_init();
curl_setopt($ch,CURLOPT_URL,"https://sheets.googleapis.com/v4/spreadsheets/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/values/Data!A2:D");
curl_setopt($ch,CURLOPT_SSL_VERIFYPEER,FALSE);
curl_setopt($ch,CURLOPT_HTTPHEADER,Array("Content-Type: application/json","Authorization: Bearer ".$access_token));
curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
$oo=json_decode(curl_exec($ch),true);
curl_close($ch);
$query=oci_parse($conn,"delete from idistat");
oci_execute($query);
oci_free_statement($query);
$nn=count($oo["values"]);
for($i=0;$i<$nn;$i++){
  $ids=$oo["values"][$i][0];
  $ide=$oo["values"][$i][1];
  $cistat=$oo["values"][$i][3];
  $eistat=$oo["values"][$i][2];
  $query=oci_parse($conn,"insert into idistat (idstart,idend,istat,sovra,eistat) values ($ids,$ide,'$cistat','','$eistat')");
  oci_execute($query);
  oci_free_statement($query);
}

echo "istatente\n";
$access_token=file_get_contents("/home/www/data/access_token");
$ch=curl_init();
curl_setopt($ch,CURLOPT_URL,"https://sheets.googleapis.com/v4/spreadsheets/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/values/Enti!A2:D");
curl_setopt($ch,CURLOPT_SSL_VERIFYPEER,FALSE);
curl_setopt($ch,CURLOPT_HTTPHEADER,Array("Content-Type: application/json","Authorization: Bearer ".$access_token));
curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
$oo=json_decode(curl_exec($ch),true);
curl_close($ch);
$query=oci_parse($conn,"delete from istatente");
oci_execute($query);
oci_free_statement($query);
$nn=count($oo["values"]);
for($i=0;$i<$nn;$i++){
  $istat=$oo["values"][$i][0];
  $ente=$oo["values"][$i][1];
  $sovra=$oo["values"][$i][3];
  $query=oci_parse($conn,"insert into istatente (istat,ente) values ('$istat','$ente')");
  oci_execute($query);
  oci_free_statement($query);
  $query=oci_parse($conn,"update idistat set sovra='$sovra' where istat='$istat'");
  oci_execute($query);
  oci_free_statement($query);
}

oci_close($conn);

?>
