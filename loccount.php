<?php

include "data.php";
$conn=oci_connect($p1,$p2,$p3);

$query=oci_parse($conn,"select count(*) from dhcpwifi");
oci_execute($query);
$row=oci_fetch_row($query);
$v2=$row[0];
oci_free_statement($query);
$query=oci_parse($conn,"select count(distinct fnv1a) from dhcpwifi");
oci_execute($query);
$row=oci_fetch_row($query);
$v3=$row[0];
oci_free_statement($query);

$tt=(int)(time()/86400);
oci_close($conn);

$vv2=$v2.",".$v3;
$rr=$tt-19995;
include "/home/www/restdati.lepida.it/googleset.php";
$access_token=file_get_contents("/home/www/data/access_token");
$curlPost='
{
  "valueInputOption": "RAW",
  "data": [
    {
      "range": "users!A'.$rr.':C'.$rr.'",
      "majorDimension": "ROWS",
      "values": [
        ['.$tt.','.$vv2.']
      ]
    }
  ]
}';
$ch = curl_init();
curl_setopt($ch,CURLOPT_URL,"https://sheets.googleapis.com/v4/spreadsheets/16bXq6V0DdIDlQEIyYlH5i424nrvI-gt_0erJSaoSMVg/values:batchUpdate");
curl_setopt($ch,CURLOPT_RETURNTRANSFER,1);
curl_setopt($ch,CURLOPT_POST,1);
curl_setopt($ch,CURLOPT_SSL_VERIFYPEER,FALSE);
curl_setopt($ch,CURLOPT_HTTPHEADER,Array("Content-Type: application/json","Authorization: Bearer ".$access_token));
curl_setopt($ch,CURLOPT_POSTFIELDS,$curlPost);
curl_exec($ch);
curl_close($ch);

?>