#!/usr/bin/php-cgi

<h3> Method GET by 'Submit'</h3>
<h3> PHP file called.</h3>

<p>You entered: </p>

<?php
    parse_str($_ENV['QUERY_STRING'], $query);
    echo "Street: [ ";
    echo $query["street"];
    echo " ] <br>";
    echo "City:   [ ";
    echo $query["city"];
    echo " ]  ";
    sleep(0.1);
?>