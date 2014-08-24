<?php

$cwd[__FILE__] = __FILE__;
if (is_link($cwd[__FILE__])) $cwd[__FILE__] = readlink($cwd[__FILE__]);
$cwd[__FILE__] = dirname($cwd[__FILE__]);

require_once($cwd[__FILE__] . "/../../citizen_science_grid/header.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/navbar.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/news.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/footer.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/my_query.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/uotd.php");

print_header("SubsetSum@Home: Progress", "", "subset_sum");
print_navbar("Projects: SubsetSum@Home", "SubsetSum@Home");


echo "
    <div class='container'>
        <div class='row'>";

include $cwd[__FILE__] . "/templates/progress.html";

echo "  </div>

        <div class='row'>";

$result = query_subset_sum_db("SELECT max_value, subset_size, slices, completed, errors, failed_set_count, webpage_generated FROM sss_runs ORDER BY max_value, subset_size");

require_once("need.php");

echo "
        </div>
    </div>";

print_footer('Travis Desell and the SubsetSum@Home Team', 'Travis Desell, Tom O\'Neil');

echo "</body></html>";


?>

