int x = 5;
x = 3 + 4 * 2 - 6 / 3;
for (int i = 0; i < 5; i = i + 1)
{
    x = x + i;
}
while (x < 20)
{
    x = x + 1;
}
do
{
    x = x - 1;
} while (x > 10);
if (x == 15)
{
    x = 0;
}
else
{
    x = 1;
}
switch (x)
{
case 0:
    x = 10;
    break;
case 1:
    x = 20;
    break;
default:
    x = 30;
}