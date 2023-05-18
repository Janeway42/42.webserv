#!/usr/bin/php-cgi

<?php
// Set the content type to plain text
// header('Content-Type: text/plain');

// Print a message to the client

print "<h3> ... This is Python script for POST method ...</h3><br>";


for ($i = 1; $i <= 20; $i++) {
    $red = rand(0, 255); $green = rand(0, 255); $blue = rand(0, 255);
    $color1 = "rgb($red, $green, $blue)";
    print   "<div style='background-color: $color1; width:20%; margin:1% 0 0 5%;     padding:1% 1% 1% 1%'>";

    $red = rand(0, 255); $green = rand(0, 255); $blue = rand(0, 255);
    $color2 = "rgb($red, $green, $blue)";
    $red = rand(0, 255); $green = rand(0, 255); $blue = rand(0, 255);
    $color3 = "rgb($red, $green, $blue)";
    print "<p style='background-color:$color2; color:$color3'> Hello from PHP script $i </p> 
    </div>";
}







// print "<span style='background-color:plum; padding:1%;'><p>Hello from Webserver PHP CGI!</p></span>";
// print "<span style='background-color:cornsilk; padding:1%;'><p>Hello from Webserver PHP CGI!</p></span>";
// print "<span style='background-color:cornsilk; padding:1%;'><p>Hello from Webserver PHP CGI!</p></span>";
// print "<div style='width:fit-content; padding:5%; margin: 3% 0% 0% 10%; border:1px solid; background-color: linen; border-radius: 4px'>";

// print("")
// print "<br>";
// sleep(1)
?>

