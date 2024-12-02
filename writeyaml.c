#include <stdio.h>
#include "KeyLogger2.c"

// Function to write results to YAML file
void writeResultsToYaml(float avgKeyPresses, float avgKeyPressesInterval, float avgEnterPresses, float avgBackspacePresses, float avgClicks, float avgLeftClicks, float avgRightClicks, float clickDifference)
{
    FILE *file = fopen("metrics.yaml", "w");
    if (file == NULL)
    {
        printf("Error opening file for writing.\n");
        return;
    }

    // Write data in YAML format
    fprintf(file, "metrics:\n");
    fprintf(file, "  average_keypresses_interval_per_15_minutes: %.2f\n", avgKeyPressesInterval);
    fprintf(file, "  average_keypresses_per_15_minutes: %.2f\n", avgKeyPresses);
    fprintf(file, "  average_enter_presses_per_15_minutes: %.2f\n", avgEnterPresses);
    fprintf(file, "  average_backspace_presses_per_15_minutes: %.2f\n", avgBackspacePresses);
    fprintf(file, "  average_mouse_clicks_per_15_minutes: %.2f\n", avgClicks);
    fprintf(file, "  average_left_mouse_clicks_per_15_minutes: %.2f\n", avgLeftClicks);
    fprintf(file, "  average_right_mouse_clicks_per_15_minutes: %.2f\n", avgRightClicks);
    fprintf(file, "  click_difference_per_15_minutes: %.2f\n", clickDifference);

    fclose(file);
    printf("Results written to metrics.yaml\n");
}