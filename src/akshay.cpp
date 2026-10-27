#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>

void bresenham(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    int e2;

    while (1)
    {
        putpixel(x1, y1, WHITE);

        if (x1 == x2 && y1 == y2)
            break;

        e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

int main()
{
    int x1, y1, x2, y2;

    printf("Enter starting point (x1 y1): ");
    scanf("%d %d", &x1, &y1);

    printf("Enter ending point (x2 y2): ");
    scanf("%d %d", &x2, &y2);

    initwindow(800, 600, "Bresenham Line Drawing");

    setcolor(YELLOW);
    outtextxy(20, 20, "Bresenham's Line Drawing Algorithm");

    bresenham(x1, y1, x2, y2);

    getch();
    closegraph();

    return 0;
}
