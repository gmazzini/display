<?php

include "/home/admgm02/data.php";
$conn=oci_connect($p1,$p2,$p3);

function mycheck($conn,$table,$istat){
  $query=oci_parse($conn,"select count(*) from $table where istat='$istat'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  @$zz=$row[0];
  oci_free_statement($query);
  return $zz; 
}

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


make3($conn,"man","man","1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE","MAN!K2:V",11,-1,"\$oo['values'][\$i][0]=='MAN'",$sovra,$ss);
make3($conn,"pal","pal","1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE","PAL!V2:V",0,-1,"1==1",$sovra,$ss);
exit(0);

make3($conn,"uiftth","uiftth","1Nk39CPjf9Lu_UQ_zUnY97cqYZ5Vh7K00owrw-XeSgHM","B4:D",0,2,"",$sovra,$ss);
make3($conn,"apwifi","apwifi","1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0","WISPER!H2:I",0,1,"",$sovra,$ss);


exit(0);



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
for($i=0;$i<$yy;$i++){
  $kk=$yyistat[$i];
  echo "userwifi:$kk\n";
  $query=oci_parse($conn,"select count(distinct id) from dhcpwifi where istat='$kk'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  if(isset($row[0]))$vv=$row[0];
  else $vv=0;
  oci_free_statement($query);
  if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
  else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
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
if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
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
  if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
  else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
  oci_execute($query);
  oci_free_statement($query);
}

function fai1($conn,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  $aux=json_decode(file_get_contents("$url"),true);
  foreach($aux["dati"] as $k => $v){
    $kk=substr($k,1,5);
    $vv=(int)$v["$field"];
    if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
    else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
    oci_execute($query);
    oci_free_statement($query);
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

function fai2($conn,$base,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  $aux=explode("\n",file_get_contents("$url"));
  for($i=$base;;$i++){
    if(!isset($aux[$i]))break;
    $aa=explode(",",$aux[$i]);
    if(strlen($aa[0])<5)continue;
    $kk=substr($aa[0],1,5);
    if(!is_numeric($kk))continue;
    if(isset($aa[1]))$vv=substr($aa[1],1,strlen($aa[1])-2);
    else $vv=1;
    @$ddd[$kk]+=(int)$vv;
  }
  foreach($ddd as $kk => $vv){
    if(mycheck($conn,$table,$kk))$query=oci_parse($conn,"update $table set $field=$vv where istat='$kk'");
    else $query=oci_parse($conn,"insert into $table (istat,$field) values ('$kk',$vv)");
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

// fai2($conn,1,"man","man","https://docs.google.com/spreadsheets/d/1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE/gviz/tq?tq=select%20V%20where%20K%3D%27MAN%27&tqx=out:csv&gid=902689105",$sovra,$ss);
// fai2($conn,1,"pal","pal","https://docs.google.com/spreadsheets/d/1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE/gviz/tq?tq=select%20V&tqx=out:csv&gid=1797209276",$sovra,$ss);
fai2($conn,1,"scuole","scuole","https://docs.google.com/spreadsheets/d/10xN81W5Dd8LRjVOm_FJ0ubzgJ1EWJ4Vfi8P3iVZdBho/gviz/tq?tq=select%20A%20where%20U%3D%27BULBUL%27&tqx=out:csv&gid=566741345",$sovra,$ss);
fai2($conn,1,"areeaai","areeaai","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20G&tqx=out:csv&gid=566741345",$sovra,$ss);
// fai2($conn,3,"uiftth","uiftth","https://docs.google.com/spreadsheets/d/1Nk39CPjf9Lu_UQ_zUnY97cqYZ5Vh7K00owrw-XeSgHM/gviz/tq?tq=select%20B%2CD&tqx=out:csv",$sovra,$ss);
fai2($conn,1,"aziendeaai","aziendeaai","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20G%2CF&tqx=out:csv&gid=566741345",$sovra,$ss);
// fai2($conn,1,"apwifi","apwifi","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20H%2CI&tqx=out:csv&gid=1373772362",$sovra,$ss);

fai1($conn,"attivifse","attivi","https://dati.fascicolo-sanitario.it/rest/attivi/comune",$sovra,$ss);
fai1($conn,"accessifse","accessi","https://dati.fascicolo-sanitario.it/rest/accessi/comune",$sovra,$ss);
fai1($conn,"scaricatifse","scaricati","https://dati.fascicolo-sanitario.it/rest/documenti/comune",$sovra,$ss);
fai1($conn,"attivazionilepidaid","attivazioni","https://dati.fascicolo-sanitario.it/rest/lepidaid/attivazioni/comune",$sovra,$ss);
fai1($conn,"accessilepidaid","accessi","https://dati.fascicolo-sanitario.it/rest/lepidaid/accessi/comune",$sovra,$ss);
fai1($conn,"sportellilepidaid","sportelli","https://dati.fascicolo-sanitario.it/rest/lepidaid/sportelli/comune",$sovra,$ss);

oci_close($conn);

function make3($conn,$table,$field,$spreadsheetid,$range,$i1,$i2,$cond,$sovra,$ss){
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
    else $vv=($i2==-1)?1:(int)$oo["values"][$i][$i2];
    @$ddd[$istat]+=(int)$vv;
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
