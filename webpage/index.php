<?php

$cwd[__FILE__] = __FILE__;
if (is_link($cwd[__FILE__])) $cwd[__FILE__] = readlink($cwd[__FILE__]);
$cwd[__FILE__] = dirname($cwd[__FILE__]);

require_once($cwd[__FILE__] . "/../../citizen_science_grid/header.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/navbar.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/news.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/footer.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/my_query.php");
require_once($cwd[__FILE__] . "/../../citizen_science_grid/csg_uotd.php");

print_header("SubsetSum@Home", "", "subset_sum");
print_navbar("Projects: SubsetSum@Home", "SubsetSum@Home", "..");

echo "
    <div class='container'>
        <div class='row'>
            <div class='col-sm-12'>
";

$carousel_info['items'][] = array(
    'image' => './images/spiral_resized.png',
    'active' => 'true',
    'text' => "<h4>The Subset Sum Problem</h4><p>The Subset Sum problem is described as follows:  given a set of positive integers <tt>S</tt> and a target sum <tt>t</tt>, is there a subset of <tt>S</tt> whose sum is <tt>t</tt>?  It is one of the well-known, so-called \"hard\" problems in computing.  It's actually a very simple problem, and the computer program to solve it is not extremely complicated.  What's hard about it is the running time – all known exact algorithms have running time that is proportional to an exponential function of the number of elements in the set (for worst-case instances of the problem).</p>");

$carousel_info['items'][] = array(
    'image' => './images/spiral_resized.png',
    'text' => "<h4>SubsetSum@Home Goals</h4><p>Over the years, a large number of combinatorial problems have been shown to be in the same class as Subset Sum  (called NP-complete problems).  But, depending on how you measure the size of the problem instance, there is evidence that Subset Sum is actually an easier problem that most of the others in its class.  The goal of this project is to strengthen the evidence that Subset Sum is an easier hard problem.</p>");

$carousel_info['items'][] = array(
    'image' => './images/spiral_resized.png',
    'text' => "<h4>The Conjecture</h4><p>Suppose we have a set of n positive whole numbers <tt>S</tt> whose maximum number is <tt>m</tt>.  We define the ratio <tt>n/m</tt> to be the density of the set and denote the sum of all elements in the set as <tt>&sum;S</tt>. If you look at the sums produced by subsets of <tt>S</tt>, you notice that very few are missing if <tt>S</tt> is dense enough.  In fact, it appears that there is an exact density threshold beyond which no sums between <tt>m</tt> and half the sum of <tt>S</tt> will be missing.  Our preliminary experiments have led to the following hypothesis:  A set of positive integers with maximum element m and size <tt>n > floor(m/2)+1</tt> has a subset with sum is <tt>t</tt> for every <tt>t</tt> in the range <tt>m < t < &sum;S − m</tt>.</p>");

$carousel_info['items'][] = array(
    'image' => './images/spiral_resized.png',
    'text' => "<h4>Help the Project</h4><p>So here's where you can help.  So far, we haven't been able to prove the hypothesis above.  If you want to be really helpful, you can send us a proof (or show us where to find one in the research literature), and the project will be done.  But if you want to be slightly less helpful and have more fun, you can volunteer your computer as a worker to see how far we can extend the empirical evidence.  You will also be helping us figure out better ways to apply distributed computing to combinatorial problems.</p>");

$projects_template = file_get_contents($cwd[__FILE__] . "/../../citizen_science_grid/templates/carousel.html");

$m = new Mustache_Engine;
echo $m->render($projects_template, $carousel_info);

echo "
            <div class='btn-group btn-group-justified' style='margin-top:20px;'>
                <a class='btn btn-primary' role='button' href='../instructions.php'><h4>Volunteer Your Computer</h4></a>
            </div>

            </div> <!-- col-sm-12 -->
        </div> <!-- row -->

        <div class='row'>
            <div class='col-sm-6'>";

show_uotd(2, 10, "style='margin-top:20px;'");
csg_show_news();

echo "
            </div> <!-- col-sm-6 -->

            <div class='col-sm-6'>";

include $cwd[__FILE__] . "/templates/subset_sum_info.html";

echo "
            </div> <!-- col-sm-6 -->
        </div> <!-- row -->
    </div> <!-- /container -->";


print_footer('Travis Desell and the SubsetSum@Home Team', 'Travis Desell, Tom O\'Neil');

echo "</body></html>";

?>
