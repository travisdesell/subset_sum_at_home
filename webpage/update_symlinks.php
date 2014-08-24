<?php

if (count($argv) != 2) {
    die("Error, invalid arguments. usage: php $argv[0] <target_directory>\n");
}

$target = $argv[1];
$cwd = dirname(__FILE__);

echo "cwd:    $cwd\n";
echo "target: $target\n";

foreach (glob("*.php") as $filename) {
    if ($filename == "boinc_db.php" || $filename == "wildlife_db.php") {
        echo "Not copying '$filename' because it contains the database passwords and is not needed.\n";
        continue;
    }

    $command = "ln -s $cwd/$filename $target/$filename";
    echo "$command\n";
    shell_exec("rm $target/$filename");
    shell_exec($command);
}

foreach (glob("*.js") as $filename) {
    //echo $filename . "\n";
    $command = "ln -s $cwd/$filename $target/$filename";
    echo "$command\n";
    shell_exec("rm $target/$filename");
    shell_exec($command);
}

foreach (glob("*.css") as $filename) {
    //echo $filename . "\n";
    $command = "ln -s $cwd/$filename $target/$filename";
    echo "$command\n";
    shell_exec("rm $target/$filename");
    shell_exec($command);
}

$command = "ln -s $cwd/css $target/css";
shell_exec("rm $target/css");
shell_exec($command);

$command = "ln -s $cwd/images $target/images";
shell_exec("rm $target/images");
shell_exec($command);

$command = "ln -s $cwd/js $target/js";
shell_exec("rm $target/js");
shell_exec($command);

$command = "ln -s $cwd/link_accounts $target/link_accounts";
shell_exec("rm $target/link_accounts");
shell_exec($command);

$command = "ln -s $cwd/templates $target/templates";
shell_exec("rm $target/templates ");
shell_exec($command);
?>
