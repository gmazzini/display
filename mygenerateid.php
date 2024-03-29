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
  $vv=$oo["values"][$i][0];
  $qq=$oo["values"][$i][1];
  $kk=$oo["values"][$i][3];
  $eistat=$oo["values"][$i][2];
  $query=oci_parse($conn,"insert into idistat (idstart,idend,istat,sovra,eistat) values ($vv,$qq,'$kk','','$eistat')");
  oci_execute($query);
  oci_free_statement($query);
}

exit(0);



echo "istatente\n";
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/gviz/tq?tq=select%20A%2CB%2CD&tqx=out:csv&gid=0"));
for($i=1;;$i++){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  if(strlen($aa[0])<5)continue;
  $kk=substr($aa[0],1,5);
  if(!is_numeric($kk))continue;
  $vv=str_replace("'","''",substr($aa[1],1,strlen($aa[1])-2));
  $qq=substr($aa[2],1,strlen($aa[2])-2);
  if(mycheck($conn,"istatente",$kk))$query=oci_parse($conn,"update istatente set ente='$vv' where istat='$kk'");
  else $query=oci_parse($conn,"insert into istatente (istat,ente) values ('$kk','$vv')");
  oci_execute($query);
  oci_free_statement($query);
  $query=oci_parse($conn,"update idistat set sovra='$qq' where istat='$kk'");
  oci_execute($query);
  oci_free_statement($query);
}

oci_close($conn);

function mycheck($conn,$table,$istat){
  $query=oci_parse($conn,"select count(*) from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$zz=$row[0];
  oci_free_statement($query);
  return $zz; 
}

function make3($conn,$table,$field,$spreadsheetid,$range,$i1,$i2,$sovra,$ss){
  echo "$table\n";
  include "/home/www/restdati.lepida.it/googleset.php";
  $access_token=file_get_contents("/home/www/data/access_token");
  echo "https://sheets.googleapis.com/v4/spreadsheets/$spreadsheetid/values/$range\n";
  $ch=curl_init();
  curl_setopt($ch,CURLOPT_URL,"https://sheets.googleapis.com/v4/spreadsheets/$spreadsheetid/values/$range");
  curl_setopt($ch,CURLOPT_SSL_VERIFYPEER,FALSE);
  curl_setopt($ch,CURLOPT_HTTPHEADER,Array("Content-Type: application/json","Authorization: Bearer ".$access_token));
  curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
  $oo=json_decode(curl_exec($ch),true);
  $nn=count($oo["values"]);
  for($i=0;$i<$nn;$i++){
    $kk=$oo["values"][$i][$i1];
    $vv=(int)$oo["values"][$i][$i2];
    @$ddd[$kk]+=(int)$vv;
  }
  curl_close($ch);
  $query=oci_parse($conn,"delete from $table");
  oci_execute($query);
  oci_free_statement($query);
  foreach($ddd as $kk => $vv){
    $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
    oci_execute($query);
  }
  $query=oci_parse($conn,"select sum($field) from $table where istat>'30000'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $vv=$row[0];
  oci_free_statement($query);
  $kk="00008";
  if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
  else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
  oci_execute($query);
  oci_free_statement($query);
  for($i=0;$i<$ss;$i++){
    $query=oci_parse($conn,"select sum($table.$field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if(isset($row[0]))$vv=$row[0];
    else $vv=0;
    oci_free_statement($query);
    $kk=$sovra[$i];
    if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
    else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
    oci_execute($query);
    oci_free_statement($query);
  }
}

?>
