import geometry;

[host]
float lengthSquared(Point p)
{
    return p.x * p.x + p.y * p.y;
}

[host]
Point translate(Point p, float dx, float dy)
{
    return Point(p.x + dx, p.y + dy);
}
