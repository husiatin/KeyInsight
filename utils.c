// Contributers: Finn Jakob

#include "utils.h"

float calculateAverage(float *data, int size)
{
    float sum = 0.0f;

    for (int i = 0; i < size; i++)
    {
        if (data[i] > 0)
        {
            sum += data[i];
        }
    }
    return (size == 0) ? 0.0f : sum / (float)size;
}
