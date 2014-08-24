<?php

echo "<div class='col-sm-12'>
        <table class='table table-condensed'>
            <thead>
                <tr>
                    <th>Max Value (M)</th>
                    <th>Subset Size (N)</th>
                    <th>Workunits</th>
                    <th>Completed</th>
                    <th>Errored</th>
                    <th>Failed Sets</th>
                    <th>Details</th>
                </tr>
            </thead>
            <tbody>";

while($row = $result->fetch_assoc()) {
    echo mysql_error();
    if ($row['max_value'] % 2 == 1) {
        echo "<tr>";
    } else {
        echo "<tr class='active'>";
    }

    echo "<td>" . $row['max_value'] . "</td>";
    echo "<td>" . $row['subset_size'] . "</td>";
    echo "<td>" . $row['slices'] . "</td>";
    echo "<td>" . $row['completed'] . "</td>";
    echo "<td>" . $row['errors'] . "</td>";
    echo "<td>" . $row['failed_set_count'] . "</td>";
    if ($row['webpage_generated'] == true) {
        echo "<td><a href=\"../subset_sum/download/set_" . $row['max_value'] . "c" . $row['subset_size'] . ".html\">details</a></td>";
    } else {
        echo "<td></td>";
    }

    echo "</tr>";
}
echo "
                </tbody>
            </table>
    </div>";
?>
