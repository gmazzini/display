<?php

include "/home/admgm02/data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select distinct sovra from idistat");
oci_execute($query);
for($ss=0;;){
  $row=oci_fetch_row($query);
  if($row==null)break;
  if(strlen($row[0])!=5)continue;
  $sovra[$ss]=$row[0];
  $ss++;
}
oci_free_statement($query);

make4($conn,"uiftth","uiftth","11EyZCYlMyiGn9GetrFq1WpOFDcHEBegSBOkzaJYmAsA","Comuni!B3:E",0,3,"",$sovra,$ss);
exit(0);

make5($conn,"attivifse","attivi","https://dati.fascicolo-sanitario.it/rest/attivi/comune",$sovra,$ss);
make5($conn,"accessifse","accessi","https://dati.fascicolo-sanitario.it/rest/accessi/comune",$sovra,$ss);
make5($conn,"scaricatifse","scaricati","https://dati.fascicolo-sanitario.it/rest/documenti/comune",$sovra,$ss);
make5($conn,"attivazionilepidaid","attivazioni","https://dati.fascicolo-sanitario.it/rest/lepidaid/attivazioni/comune",$sovra,$ss);
make5($conn,"accessilepidaid","accessi","https://dati.fascicolo-sanitario.it/rest/lepidaid/accessi/comune",$sovra,$ss);
make5($conn,"sportellilepidaid","sportelli","https://dati.fascicolo-sanitario.it/rest/lepidaid/sportelli/comune",$sovra,$ss);

make4($conn,"aziendeaai","aziendeaai","1_H9hzrPmySAfAMkLtGmH5IIB8cric6OtUwXf3Jfl6ME","Aziende!B2:B",0,-1,"1==1",$sovra,$ss);
make4($conn,"areeaai","areeaai","1_H9hzrPmySAfAMkLtGmH5IIB8cric6OtUwXf3Jfl6ME","Nodi!B2:B",0,-1,"1==1",$sovra,$ss);
make4($conn,"apwifi","apwifi","1ZMxZbD-gGhLGR5RdcBHhRsZQeARiRrK1Nw-2PSWHbZE","Apparati!AN2:AO",0,1,"",$sovra,$ss);
make4($conn,"scuole","scuole","1SUGh7fL0zkppRbYZykrdVoim5XLKv9Vwt1yDstLPu-o","Elenco!N2:U",0,-1,"@\$oo['values'][\$i][7]=='BULBUL'",$sovra,$ss);
make4($conn,"man","man","1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE","MAN!K2:V",11,-1,"\$oo['values'][\$i][0]=='MAN'",$sovra,$ss);
make4($conn,"pal","pal","1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE","PAL!V2:V",0,-1,"1==1",$sovra,$ss);

$query=oci_parse($conn,"select distinct istat from idistat where istat>'30000'");
oci_execute($query);
for($yy=0;;){
  $row=oci_fetch_row($query);
  if($row==null)break;
  if(strlen($row[0])!=5)continue;
  $yyistat[$yy]=$row[0];
  $yy++;
}
oci_free_statement($query);

echo "userwifi\n";
$tt=(int)(time()/86400)-365;
$query=oci_parse($conn,"delete from dhcpwifi where tt<$tt");
oci_execute($query);
oci_free_statement($query);
$table="userwifi";
$field="userwifi";
$tt=(int)(time()/86400);
$query=oci_parse($conn,"delete from $table where tt=$tt");
oci_execute($query);
oci_free_statement($query);
for($i=0;$i<$yy;$i++){
  $kk=$yyistat[$i];
  echo "userwifi:$kk\n";
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where istat='$kk'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  if(isset($row[0]))$vv=$row[0];
  else $vv=0;
  oci_free_statement($query);
  $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
  oci_execute($query);
  oci_free_statement($query);
}
$kk="00008";
echo "userwifi:$kk\n";
$query=oci_parse($conn,"select count(distinct id) from dhcpwifi");
oci_execute($query);
$row=oci_fetch_row($query);
$vv=$row[0];
oci_free_statement($query);
$query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
oci_execute($query);
oci_free_statement($query);
for($i=0;$i<$ss;$i++){
  $kk=$sovra[$i];
  echo "userwifi:$kk\n";
  $query=oci_parse($conn,"select count(distinct dhcpwifi.id) from dhcpwifi,idistat where dhcpwifi.istat=idistat.istat and idistat.sovra='$kk'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  if(isset($row[0]))$vv=$row[0];
  else $vv=0;
  oci_free_statement($query);
  $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
  oci_execute($query);
  oci_free_statement($query);
}

oci_close($conn);

function make4($conn,$table,$field,$spreadsheetid,$range,$i1,$i2,$cond,$sovra,$ss){
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
    @$istat=$oo["values"][$i][$i1];
    if(strlen($istat)!=5)continue;
    if($i2==-1)eval("if($cond)\$vv=1; else \$vv=0;");
    else @$vv=(int)$oo["values"][$i][$i2];
    @$ddd[$istat]+=(int)$vv;
  }
  curl_close($ch);
  $tt=(int)(time()/86400);
  $query=oci_parse($conn,"delete from $table where tt=$tt");
  oci_execute($query);
  oci_free_statement($query);
  foreach($ddd as $kk => $vv){
    $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
    oci_execute($query);
  }
  $query=oci_parse($conn,"select sum($field) from $table where istat>'30000' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $vv=$row[0];
  oci_free_statement($query);
  $kk="00008";
  $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
  oci_execute($query);
  oci_free_statement($query);
  for($i=0;$i<$ss;$i++){
    $query=oci_parse($conn,"select sum($table.$field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]' and $table.tt=$tt");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if(isset($row[0]))$vv=$row[0];
    else $vv=0;
    oci_free_statement($query);
    $kk=$sovra[$i];
    $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
    oci_execute($query);
    oci_free_statement($query);
  }
}

function make5($conn,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  $tt=(int)(time()/86400);
  $query=oci_parse($conn,"delete from $table where tt=$tt");
  oci_execute($query);
  oci_free_statement($query);
  $aux=json_decode(file_get_contents("$url"),true);
  foreach($aux["dati"] as $k => $v){
    $kk=substr($k,1,5);
    $vv=(int)$v["$field"];
    $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
    oci_execute($query);
    oci_free_statement($query);
  }
  $query=oci_parse($conn,"select sum($field) from $table where istat>'30000' and tt=$tt");
  oci_execute($query);
  $row=oci_fetch_row($query);
  $vv=$row[0];
  oci_free_statement($query);
  $kk="00008";
  $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
  oci_execute($query);
  oci_free_statement($query);
  for($i=0;$i<$ss;$i++){
    $query=oci_parse($conn,"select sum($table.$field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]' and $table.tt=$tt");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if(isset($row[0]))$vv=$row[0];
    else $vv=0;
    oci_free_statement($query);
    $kk=$sovra[$i];
    $query=oci_parse($conn,"insert into $table (istat,$field,tt) values ('$kk',$vv,$tt)");
    oci_execute($query);
    oci_free_statement($query);
  }
}

?>
