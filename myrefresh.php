<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

echo "istatente\n";
$query=oci_parse($conn,"delete from istatente");
oci_execute($query);
$aux=explode("\n",file_get_contents("https://docs.google.com/spreadsheets/d/1DTngQUDqQgcYhA4S1iOW3jGuj-nOO-98opbXeUC-ffA/gviz/tq?tq=select%20A%2CB%2CD&tqx=out:csv&gid=0"));
for($i=1;;$i++){
  if(!isset($aux[$i]))break;
  $aa=explode(",",$aux[$i]);
  if(strlen($aa[0])<5)continue;
  $kk=substr($aa[0],1,5);
  if(!is_numeric($kk))continue;
  $vv=str_replace("'","''",substr($aa[1],1,strlen($aa[1])-2));
  $qq=substr($aa[2],1,strlen($aa[2])-2);
  $query=oci_parse($conn,"insert into istatente values ('$kk','$vv')");
  oci_execute($query);
  $query=oci_parse($conn,"update idistat set sovra='$qq' where istat='$kk'");
  oci_execute($query);
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
$query=oci_parse($conn,"delete from userwifi");
oci_execute($query);
for($i=0;$i<$yy;$i++){
  $query=oci_parse($conn,"select count(distinct id) from logwifi where istat='$yyistat[$i]'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  if(isset($row[0]))$vv=$row[0];
  else $vv=0;
  oci_free_statement($query);
  echo "userwifi:$yyistat[$i]\n";
  $query=oci_parse($conn,"insert into userwifi values ('$yyistat[$i]',$vv)");
  oci_execute($query);
}
echo "userwifi:00008\n";
$query=oci_parse($conn,"select count(distinct id) from logwifi");
oci_execute($query);
$row=oci_fetch_row($query);
$vv=$row[0];
oci_free_statement($query);
$query=oci_parse($conn,"insert into userwifi values ('00008',$vv)");
oci_execute($query);
for($i=0;$i<$ss;$i++){
  echo "userwifi:$sovra[$i]\n";
  $query=oci_parse($conn,"select count(distinct logwifi.id) from logwifi,idistat where logwifi.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
  oci_execute($query);
  $row=oci_fetch_row($query);
  if(isset($row[0]))$vv=$row[0];
  else $vv=0;
  oci_free_statement($query);
  $query=oci_parse($conn,"insert into userwifi values ('$sovra[$i]',$vv)");
  oci_execute($query);
}

function fai1($conn,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  $query=oci_parse($conn,"delete from $table");
  oci_execute($query);
  $aux=json_decode(file_get_contents("$url"),true);
  foreach($aux["dati"] as $k => $v){
    $kk=substr($k,1,5);
    $vv=$v["$field"];
    $query=oci_parse($conn,"insert into $table values ('$kk',$vv)");
    oci_execute($query);
  }
  $query=oci_parse($conn,"insert into $table select '00008',sum($field) from $table");
  oci_execute($query);
  for($i=0;$i<$ss;$i++){
    $query=oci_parse($conn,"select sum($table.$field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if(isset($row[0]))$vv=$row[0];
    else $vv=0;
    oci_free_statement($query);
    $query=oci_parse($conn,"insert into $table values ('$sovra[$i]',$vv)");
    oci_execute($query);
  }
}

function fai2($conn,$base,$table,$field,$url,$sovra,$ss){
  echo "$table\n";
  $query=oci_parse($conn,"delete from $table");
  oci_execute($query);
  $aux=explode("\n",file_get_contents("$url"));
  for($i=$base;;$i++){
    if(!isset($aux[$i]))break;
    $aa=explode(",",$aux[$i]);
    if(strlen($aa[0])<5)continue;
    $kk=substr($aa[0],1,5);
    if(!is_numeric($kk))continue;
    if(isset($aa[1]))$vv=substr($aa[1],1,strlen($aa[1])-2);
    else $vv=1;
    $query=oci_parse($conn,"select count(*) from $table where istat='$kk'");
    oci_execute($query);
    $row=oci_fetch_row($query);
    @$myexist=$row[0];
    oci_free_statement($query);
    if(!$myexist){
      $query=oci_parse($conn,"insert into $table values ('$kk',0)");
      oci_execute($query);
    }
    $query=oci_parse($conn,"update $table set $field=$field+$vv where istat='$kk'");
    oci_execute($query);
  }
  $query=oci_parse($conn,"insert into $table select '00008',sum($field) from $table");
  oci_execute($query);
  for($i=0;$i<$ss;$i++){
    $query=oci_parse($conn,"select sum($table.$field) from $table,idistat where $table.istat=idistat.istat and idistat.sovra='$sovra[$i]'");
    oci_execute($query);
    $row=oci_fetch_row($query);
    if(isset($row[0]))$vv=$row[0];
    else $vv=0;
    oci_free_statement($query);
    $query=oci_parse($conn,"insert into $table values ('$sovra[$i]',$vv)");
    oci_execute($query);
  }
}

fai2($conn,1,"man","man","https://docs.google.com/spreadsheets/d/1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE/gviz/tq?tq=select%20V%20where%20K%3D%27MAN%27&tqx=out:csv&gid=902689105",$sovra,$ss);
fai2($conn,1,"pal","pal","https://docs.google.com/spreadsheets/d/1DEs7yoAfJ6wK9L-V5kYoArEeP-g110NgPMU0DDFv9EE/gviz/tq?tq=select%20V&tqx=out:csv&gid=1797209276",$sovra,$ss);
fai2($conn,1,"scuole","scuole","https://docs.google.com/spreadsheets/d/10xN81W5Dd8LRjVOm_FJ0ubzgJ1EWJ4Vfi8P3iVZdBho/gviz/tq?tq=select%20A%20where%20U%3D%27BULBUL%27&tqx=out:csv&gid=566741345",$sovra,$ss);
fai2($conn,1,"areeaai","areeaai","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20G&tqx=out:csv&gid=566741345",$sovra,$ss);
fai2($conn,3,"uiftth","uiftth","https://docs.google.com/spreadsheets/d/1Nk39CPjf9Lu_UQ_zUnY97cqYZ5Vh7K00owrw-XeSgHM/gviz/tq?tq=select%20B%2CD&tqx=out:csv",$sovra,$ss);
fai2($conn,1,"aziendeaai","aziendeaai","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20G%2CF&tqx=out:csv&gid=566741345",$sovra,$ss);
fai2($conn,1,"apwifi","apwifi","https://docs.google.com/spreadsheets/d/1cgCtacbWsm7wybTp8cA7wWBFo9bZOSc7JAnlV99K-O0/gviz/tq?tq=select%20H%2CI&tqx=out:csv&gid=1373772362",$sovra,$ss);

fai1($conn,"attivifse","attivi","https://dati.fascicolo-sanitario.it/rest/attivi/comune",$sovra,$ss);
fai1($conn,"accessifse","accessi","https://dati.fascicolo-sanitario.it/rest/accessi/comune",$sovra,$ss);
fai1($conn,"scaricatifse","scaricati","https://dati.fascicolo-sanitario.it/rest/documenti/comune",$sovra,$ss);
fai1($conn,"attivazionilepidaid","attivazioni","https://dati.fascicolo-sanitario.it/rest/lepidaid/attivazioni/comune",$sovra,$ss);
fai1($conn,"accessilepidaid","accessi","https://dati.fascicolo-sanitario.it/rest/lepidaid/accessi/comune",$sovra,$ss);
fai1($conn,"sportellilepidaid","sportelli","https://dati.fascicolo-sanitario.it/rest/lepidaid/sportelli/comune",$sovra,$ss);

oci_close($conn);

?>