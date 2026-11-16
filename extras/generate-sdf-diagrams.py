"""Generate the tutorial's SVG vector diagrams with the Python standard library.

Run from any directory: python extras/generate-sdf-diagrams.py
Distances, reflection angles, tracing steps, and union contours follow the tutorial's math.
"""

from html import escape
import math
from pathlib import Path


COLORS = {
    "ink": "#26382f",
    "muted": "#5b6d62",
    "normal": "#387044",
    "incoming": "#b25b2b",
    "outgoing": "#286c9b",
    "error": "#a54145",
    "surface": "#e8eee7",
    "guide": "#9baa9e",
}


def along(origin, direction, length):
    """Project a world-space vector onto the drawing, where image y points down."""
    return (origin[0] + direction[0] * length, origin[1] - direction[1] * length)


def polar(origin, radius, degrees):
    """Place a point at an angle measured clockwise from the upward normal."""
    angle = math.radians(degrees)
    return along(origin, (math.sin(angle), math.cos(angle)), radius)


def pair(point):
    return f"{point[0]:.2f},{point[1]:.2f}"


class Diagram:
    """Keep typography, arrowheads, colours, and accessibility consistent across figures."""

    def __init__(self, height, title, description):
        self.parts = [
            f'<svg xmlns="http://www.w3.org/2000/svg" width="640" height="{height}" '
            f'viewBox="0 0 640 {height}" role="img" aria-labelledby="title description">',
            f'<title id="title">{escape(title)}</title>',
            f'<desc id="description">{escape(description)}</desc>',
            '<defs>',
        ]
        for name in ("normal", "incoming", "outgoing", "error"):
            self.parts.append(
                f'<marker id="{name}" viewBox="0 0 10 10" refX="9" refY="5" '
                'markerWidth="9" markerHeight="9" markerUnits="userSpaceOnUse" orient="auto">'
                f'<path d="M 0 0 L 10 5 L 0 10 Z" fill="{COLORS[name]}"/></marker>'
            )
        self.parts.extend([
            '</defs>',
            '<g font-family="Segoe UI, Arial, sans-serif" font-size="20" '
            f'fill="{COLORS["ink"]}" stroke-linecap="round" stroke-linejoin="round">',
        ])
        self.text(320, 38, title, size=24, weight=600, anchor="middle")

    def text(self, x, y, value, color="ink", size=20, weight=400, anchor="start"):
        self.parts.append(
            f'<text x="{x}" y="{y}" fill="{COLORS[color]}" font-size="{size}" '
            f'font-weight="{weight}" text-anchor="{anchor}">{escape(value)}</text>'
        )

    def line(self, start, end, color="guide", width=2, dashed=False, arrow=False):
        dash = ' stroke-dasharray="6 6"' if dashed else ''
        marker = f' marker-end="url(#{color})"' if arrow else ''
        self.parts.append(
            f'<path d="M {pair(start)} L {pair(end)}" stroke="{COLORS[color]}" '
            f'stroke-width="{width}" fill="none"{dash}{marker}/>'
        )

    def dot(self, point, color="ink", radius=5):
        self.parts.append(
            f'<circle cx="{point[0]}" cy="{point[1]}" r="{radius}" '
            f'fill="{COLORS[color]}" stroke="#fafcf9" stroke-width="2"/>'
        )

    def circle(self, point, radius, color="guide", fill="none", dashed=False):
        dash = ' stroke-dasharray="5 6"' if dashed else ''
        self.parts.append(
            f'<circle cx="{point[0]:.2f}" cy="{point[1]:.2f}" r="{radius:.2f}" '
            f'fill="{fill}" stroke="{COLORS[color]}" stroke-width="1.5"{dash}/>'
        )

    def angle(self, origin, radius, start, end):
        self.parts.append(
            f'<path d="M {pair(polar(origin, radius, start))} A {radius} {radius} '
            f'0 0 {int(end > start)} {pair(polar(origin, radius, end))}" '
            f'fill="none" stroke="{COLORS["muted"]}" stroke-width="1.5"/>'
        )

    def camera_icon(self, lens, degrees=0):
        """Draw a small side-view camera whose lens opening is at `lens`.

        The body sits behind the lens, and `degrees` turns the camera clockwise from
        facing right, so rays drawn from `lens` leave through the front of the icon.
        A camera facing left is mirrored so its viewfinder stays on top.
        """
        x, y = lens
        transform = f"rotate({degrees:.2f} {x:.2f} {y:.2f})"
        if abs(degrees) > 90:
            transform += f" translate(0 {2 * y:.2f}) scale(1 -1)"
        self.parts.extend([
            f'<g fill="#e7ede8" stroke="{COLORS["ink"]}" stroke-width="1.5" '
            f'transform="{transform}">',
            f'<rect x="{x - 44:.2f}" y="{y - 18:.2f}" width="12" height="6" rx="1.5"/>',
            f'<rect x="{x - 52:.2f}" y="{y - 13:.2f}" width="38" height="26" rx="4"/>',
            f'<path d="M {pair((x - 14, y - 5))} L {pair((x - 1, y - 12))} '
            f'L {pair((x - 1, y + 12))} L {pair((x - 14, y + 5))} Z"/>',
            f'<circle cx="{x - 39:.2f}" cy="{y - 4:.2f}" r="2.5" fill="{COLORS["ink"]}" stroke="none"/>',
            '</g>',
        ])

    def surface(self, y, depth=42):
        self.parts.append(f'<path d="M 48 {y} H 592 V {y + depth} H 48 Z" fill="{COLORS["surface"]}"/>')
        self.line((48, y), (592, y), "muted")

    def save(self, name):
        root = Path(__file__).resolve().parents[1] / "docs/heterogeneity/images"
        (root / name).write_text("\n".join(self.parts + ['</g>', '</svg>', '']), encoding="utf-8")


def signed_distance():
    figure = Diagram(435, "Measure distance to the surface",
        "A cross-section of a unit sphere. A is halfway from the centre to the surface, "
        "B is on the surface, and C is two units from the centre. Their signed distances "
        "are minus one half, zero, and plus one. The radius is one unit.")
    figure.text(320, 68, "A cross-section of a sphere with R = 1", "muted", size=18, anchor="middle")
    centre, radius = (215, 225), 110
    inside = along(centre, (1, 0), 0.5 * radius)
    boundary = along(centre, (1, 0), radius)
    outside = along(centre, (1, 0), 2 * radius)
    figure.circle(centre, radius, "muted", COLORS["surface"])
    figure.line((centre[0] - radius, centre[1]), outside, "guide", width=1)
    figure.line(centre, (centre[0] - radius, centre[1]), "normal", width=2, arrow=True)
    figure.text(159, 210, "R = 1", "normal", size=18, anchor="middle")
    for start, end, color, label in [
        (inside, boundary, "incoming", "0.5"),
        (boundary, outside, "outgoing", "1"),
    ]:
        figure.line((start[0], 188), (end[0], 188), color)
        for x in (start[0], end[0]):
            figure.line((x, 183), (x, 193), color, width=1.5)
        figure.text((start[0] + end[0]) / 2, 177, label, color, size=18, anchor="middle")
    figure.dot(centre, radius=4)
    figure.text(206, 249, "0", "muted", size=18, anchor="end")
    for point, color, label in [(inside, "incoming", "A"), (boundary, "normal", "B"),
                                (outside, "outgoing", "C")]:
        figure.dot(point, color)
        label_x = point[0] + 15 if label == "B" else point[0]
        figure.text(label_x, 251, label, color, anchor="middle")
    # Measure centre-to-C separately from the coloured surface-distance segments above.
    for x in (centre[0], outside[0]):
        figure.line((x, 266), (x, 354), "guide", width=1, dashed=True)
        figure.line((x, 349), (x, 359), "muted", width=1.5)
    figure.line((centre[0], 354), (outside[0], 354), "muted", width=1.5)
    figure.text(325, 380, "length(C) = 2", "muted", size=18, anchor="middle")
    figure.text(138, 416, "A: D = −0.5", "incoming", anchor="middle")
    figure.text(320, 416, "B: D = 0", "normal", anchor="middle")
    figure.text(502, 416, "C: D = +1", "outgoing", anchor="middle")
    figure.save("sdf-signed-distance.svg")


def sphere_tracing():
    figure = Diagram(475, "Each distance gives a safe step",
        "A horizontal ray starts at p0 outside a circular cross-section of the object. "
        "Empty circles at p0, p1, and p2 have radii equal to the signed distance. Each "
        "step advances to the next circle's centre. The nearest surface point to p0 is "
        "below the ray, illustrating why a safe step need not reach the object.")
    figure.text(320, 66, "Circles are cross-sections of empty 3D spheres.", "muted", size=18, anchor="middle")
    centre, radius = (490, 325), 105
    origin = (235, 240)
    samples = []
    x = origin[0]
    for _ in range(3):
        distance = math.hypot(x - centre[0], origin[1] - centre[1]) - radius
        samples.append(((x, origin[1]), distance))
        x += distance
    figure.circle(centre, radius, "muted", COLORS["surface"])
    for point, distance in samples:
        figure.circle(point, distance, "outgoing", dashed=True)
    hit = (centre[0] - math.sqrt(radius ** 2 - (origin[1] - centre[1]) ** 2), origin[1])
    # The nearest point is radial from the object's centre, not the ray intersection.
    centre_distance = math.hypot(origin[0] - centre[0], origin[1] - centre[1])
    nearest = tuple(c + radius * (p - c) / centre_distance for p, c in zip(origin, centre))
    figure.line(origin, nearest, "incoming", width=2, dashed=True)
    figure.text(303, 291, "D(p₀)", "incoming", size=18, anchor="middle")
    figure.dot(nearest, "incoming", radius=4)
    figure.line(nearest, (330, 331), "incoming", width=1)
    figure.text(345, 356, "nearest surface", "incoming", size=18, anchor="end")
    figure.line(origin, hit, "outgoing", width=2)
    middle = ((origin[0] + samples[1][0][0]) / 2, origin[1])
    figure.line(origin, middle, "outgoing", width=3, arrow=True)
    figure.text(296, 221, "ray direction d", "outgoing", size=18, anchor="middle")
    for point, _ in samples:
        figure.dot(point, "outgoing", radius=4)
    figure.dot(hit, radius=3)
    figure.text(226, 267, "p₀", "outgoing", anchor="end")
    for label, index, label_x, label_y in [("p₁", 1, 377, 184), ("p₂", 2, 424, 182)]:
        point = samples[index][0]
        figure.line((label_x, label_y + 8), (point[0], point[1] - 8), "guide", width=1)
        figure.text(label_x, label_y, label, "outgoing", anchor="middle")
    figure.text(453, 220, "hit", size=18)
    figure.text(525, 377, "object", "muted", size=18, anchor="middle")
    figure.text(320, 456, "Advance along the ray by the nearest surface distance.", size=18, anchor="middle")
    figure.save("sdf-sphere-tracing.svg")


def camera():
    figure = Diagram(445, "Position, aim, and field of view",
        "Side view of the camera, drawn as a small camera icon whose lens sits at the "
        "position. Position is the origin of every ray. The target "
        "lies on the forward viewing axis, beyond a virtual image plane. Focal length "
        "measures the distance from the camera to that plane in normalized image units, "
        "not the distance to the target. A blue ray passes through a pixel sample. "
        "Dashed lines through the top and bottom edges show the vertical field of view.")
    figure.text(320, 68, "Side view of a pinhole camera", "muted", size=18, anchor="middle")
    origin, plane_centre, target = (100, 230), (330, 230), (550, 230)
    half_height = 110
    top, bottom = (330, 120), (330, 340)
    sample = (330, 175)
    ray_end = (570, origin[1] + (sample[1] - origin[1])
               * (570 - origin[0]) / (sample[0] - origin[0]))

    for edge in (top, bottom):
        figure.line(origin, edge, "guide", width=1.5, dashed=True)
    figure.line(top, bottom, "incoming", width=3)
    figure.text(330, 100, "virtual image plane", "incoming", size=18, anchor="middle")
    figure.line(origin, target, "guide", width=1, dashed=True)
    figure.line(origin, (500, 230), "normal", width=2, arrow=True)
    figure.text(430, 260, "forward", "normal", size=18, anchor="middle")
    figure.dot(target, "normal")
    figure.text(550, 211, "target", "normal", anchor="middle")

    figure.line(origin, ray_end, "outgoing", width=3, arrow=True)
    figure.dot(sample, "outgoing")
    figure.text(490, 108, "camera ray", "outgoing", size=18, anchor="middle")
    figure.line(sample, (352, 191), "outgoing", width=1)
    figure.text(359, 204, "pixel sample", "outgoing", size=18)
    figure.camera_icon(origin)
    figure.dot(origin)
    figure.text(92, 200, "position", anchor="end")
    figure.text(92, 266, "ray origin", "muted", size=16, anchor="end")

    # A normalized image has half-height one, regardless of the output resolution.
    figure.line((310, plane_centre[1]), (310, plane_centre[1] + half_height), "muted", width=1)
    for y in (plane_centre[1], plane_centre[1] + half_height):
        figure.line((305, y), (315, y), "muted", width=1)
    figure.text(299, 290, "1", "muted", size=18, anchor="end")
    # The origin's guide starts below the camera icon's lens rather than through it.
    for x, y, gap in ((*origin, 16), (*bottom, 8)):
        figure.line((x, y + gap), (x, 375), "guide", width=1, dashed=True)
        figure.line((x, 370), (x, 380), "incoming", width=1.5)
    figure.line((origin[0], 375), (plane_centre[0], 375), "incoming", width=2)
    figure.text(215, 400, "focalLength", "incoming", anchor="middle")
    figure.text(320, 434, "A longer focalLength narrows the view.", "muted", size=18, anchor="middle")
    figure.save("sdf-camera.svg")


def normal_light():
    figure = Diagram(390, "The normal tells us which way a surface faces",
        "A point p on a curved surface has an outward unit normal n. The unit vector ell "
        "points from p toward the light. Theta is their angle. The normal is perpendicular "
        "to the tangent, and the clamped dot product gives the diffuse lighting factor.")
    point = (320, 245)
    start = polar((320, 465), 220, -55)
    end = polar((320, 465), 220, 55)
    figure.parts.append(
        f'<path d="M {pair(start)} A 220 220 0 0 1 {pair(end)} L {end[0]:.2f} 352 '
        f'L {start[0]:.2f} 352 Z" fill="{COLORS["surface"]}"/>'
    )
    figure.parts.append(
        f'<path d="M {pair(start)} A 220 220 0 0 1 {pair(end)}" '
        f'fill="none" stroke="{COLORS["muted"]}" stroke-width="2"/>'
    )
    figure.line((220, 245), (440, 245), "guide", width=1, dashed=True)
    figure.text(452, 250, "tangent", "muted", size=18)
    figure.line((320, 233), (332, 233), "guide", width=1)
    figure.line((332, 233), (332, 245), "guide", width=1)
    figure.line(point, (320, 85), "normal", width=3, arrow=True)
    figure.line(point, polar(point, 180, -48), "outgoing", width=3, arrow=True)
    figure.angle(point, 62, 0, -48)
    figure.dot(point)
    figure.text(338, 273, "p")
    figure.text(337, 111, "normal n", "normal")
    figure.text(101, 102, "toward light ℓ", "outgoing")
    figure.text(283, 170, "θ", anchor="middle")
    figure.text(320, 378, "Facing the light gives a larger dot product.", size=18, anchor="middle")
    figure.save("sdf-normal-light.svg")


def union_outline(blend_width):
    """Locate the zero contour of the tutorial's two overlapping spheres in a 2D slice.

    The origin lies inside both unit spheres at x = +/-0.9. Each radial line exits
    the combined shape once, so bisection locates its boundary without drawing an
    approximate hand-shaped join. The outer bracket, radius 3, is outside both fields.
    """
    contour = []
    for index in range(256):
        angle = 2 * math.pi * index / 256
        dx, dy = math.cos(angle), math.sin(angle)
        low, high = 0.0, 3.0
        for _ in range(40):
            radius = (low + high) / 2
            x, y = radius * dx, radius * dy
            a, b = math.hypot(x + 0.9, y) - 1, math.hypot(x - 0.9, y) - 1
            distance = min(a, b)
            if blend_width > 0:
                h = max(blend_width - abs(a - b), 0) / blend_width
                distance -= 0.25 * blend_width * h * h
            if distance < 0:
                low = radius
            else:
                high = radius
        contour.append((radius * dx, radius * dy))
    return contour


def smooth_union():
    figure = Diagram(355, "A smooth union rounds the join",
        "Zero contours of two unit spheres centred at x equals minus and plus 0.9. "
        "The ordinary union has a crease where the circles intersect. The smooth union "
        "with blend width 0.6 expands and rounds that join. Dashed circles show the same "
        "input spheres in both views.")
    scale = 62
    for centre, width, title, annotation, color in [
        ((174, 221), 0.0, "Ordinary union", "sharp join", "muted"),
        ((466, 221), 0.6, "Smooth union", "rounded join", "outgoing"),
    ]:
        figure.text(centre[0], 86, title, color, anchor="middle")
        contour = [along(centre, point, scale) for point in union_outline(width)]
        commands = "M " + " L ".join(pair(point) for point in contour) + " Z"
        figure.parts.append(
            f'<path d="{commands}" fill="{COLORS["surface"]}" '
            f'stroke="{COLORS[color]}" stroke-width="2.5"/>'
        )
        for offset in (-0.9, 0.9):
            figure.circle((centre[0] + offset * scale, centre[1]), scale, dashed=True)
        # At x=0 the two distances agree, so the smooth-min correction is width/4.
        join_height = math.sqrt((1 + width / 4) ** 2 - 0.9 ** 2)
        join = along(centre, (0, join_height), scale)
        figure.line((centre[0], 139), (join[0], join[1] - 7), "incoming", width=1)
        figure.dot(join, "incoming", radius=4)
        figure.text(centre[0], 126, annotation, "incoming", size=18, anchor="middle")
    figure.text(320, 316, "Same sphere positions and radii in both views.", size=18, anchor="middle")
    figure.text(320, 344, "Dashed circles show the original spheres.", "muted", size=18, anchor="middle")
    figure.save("sdf-smooth-union.svg")


def mirror():
    figure = Diagram(370, "One incoming ray, one reflected ray",
        "An incoming ray d points down and right to a surface point p. The outward normal n "
        "points up. The reflected ray r points up and right. The two angles from the normal "
        "to the ray segments above the surface are equal.")
    point = (320, 270)
    normal = (0.0, 1.0)
    angle = math.radians(48)
    incoming = (math.sin(angle), -math.cos(angle))
    dot = sum(a * b for a, b in zip(incoming, normal))
    reflected = tuple(d - 2 * dot * n for d, n in zip(incoming, normal))
    figure.surface(point[1])
    # Put the incoming arrowhead halfway along the ray so the hit-point dot cannot hide it.
    middle = along(point, incoming, -120)
    figure.line(middle, point, "incoming", width=3)
    figure.line(along(point, incoming, -240), middle, "incoming", width=3, arrow=True)
    figure.line(point, along(point, reflected, 240), "outgoing", width=3, arrow=True)
    figure.line(point, along(point, normal, 188), "normal", width=3, arrow=True)
    figure.angle(point, 57, 0, -48)
    figure.angle(point, 57, 0, 48)
    figure.text(277, 201, "θᵢ", anchor="middle")
    figure.text(363, 201, "θᵣ", anchor="middle")
    figure.text(87, 91, "incoming d", "incoming")
    figure.text(448, 91, "reflected r", "outgoing")
    figure.text(338, 112, "normal n", "normal")
    figure.dot(point)
    figure.text(320, 298, "p", anchor="middle")
    figure.text(66, 298, "surface", "muted", size=18)
    figure.text(320, 344, "θᵢ = θᵣ   ·   Both angles are measured from the normal.", size=18, anchor="middle")
    figure.save("sdf-mirror-vectors.svg")


def reflection_path():
    figure = Diagram(470, "One path, traced backwards",
        "A viewing ray leaves the camera and hits the upper-left of a sphere. The mirror "
        "bounce sends a reflected ray up and to the right until it reaches the distant "
        "environment, drawn as a dashed arc. The path has two ray segments and one "
        "surface bounce.")
    figure.text(320, 68, "From the camera to the light it sees", "muted", size=18, anchor="middle")
    centre, radius, environment = (330, 310), 90, 210
    # The reflected direction is the tutorial's mirror formula, r = d - 2(d·n)n.
    view = (0.981, -0.196)
    normal = (-0.371, 0.928)
    length = math.hypot(*view)
    view = (view[0] / length, view[1] / length)
    length = math.hypot(*normal)
    normal = (normal[0] / length, normal[1] / length)
    dot = sum(a * b for a, b in zip(view, normal))
    reflected = tuple(d - 2 * dot * n for d, n in zip(view, normal))
    hit = along(centre, normal, radius)
    camera = along(hit, view, -220)
    # The reflected ray ends where it meets the environment circle around the sphere.
    offset = (hit[0] - centre[0], centre[1] - hit[1])
    b = sum(o * r for o, r in zip(offset, reflected))
    reach = -b + math.sqrt(b * b - (radius ** 2 - environment ** 2))
    escape = along(hit, reflected, reach)

    start, end = math.radians(55), math.radians(130)
    figure.parts.append(
        f'<path d="M {pair(along(centre, (math.cos(start), math.sin(start)), environment))} '
        f'A {environment} {environment} 0 0 0 '
        f'{pair(along(centre, (math.cos(end), math.sin(end)), environment))}" '
        'fill="none" stroke="#9bb7c8" stroke-width="3" stroke-dasharray="8 7"/>'
    )
    figure.text(462, 132, "environment", "outgoing", anchor="start")
    figure.text(462, 154, "infinitely far away", "muted", size=16, anchor="start")
    figure.circle(centre, radius, "muted", COLORS["surface"])
    figure.text(centre[0], centre[1] + 30, "sphere", "muted", size=18, anchor="middle")

    figure.line(camera, hit, "incoming", width=3, arrow=True)
    figure.line(hit, escape, "outgoing", width=3, arrow=True)
    figure.camera_icon(camera, math.degrees(math.atan2(-view[1], view[0])))
    figure.dot(hit)
    figure.text(185, 190, "1  viewing ray", "incoming", size=18, anchor="middle")
    figure.text(352, 170, "2  reflected ray", "outgoing", size=18)
    # The label sits in the open wedge between the arriving and departing rays.
    figure.text(hit[0] - 14, hit[1] - 30, "bounce", size=18, anchor="middle")
    figure.text(camera[0] - 26, camera[1] + 42, "camera", "muted", size=18, anchor="middle")
    figure.text(320, 446, "Two ray segments and one surface bounce.", size=18, anchor="middle")
    figure.save("sdf-reflection-path.svg")


def hemisphere():
    figure = Diagram(420, "Directions above a surface point",
        "A two-dimensional cross-section of a hemisphere above the tangent plane. Several "
        "sampled directions point outward from p. The surface normal n points up. Theta "
        "is the angle between n and one highlighted sampled direction.")
    figure.text(320, 68, "A cross-section of the 3D hemisphere", "muted", size=18, anchor="middle")
    point = (320, 310)
    figure.surface(point[1])
    figure.parts.append(
        '<path d="M 120 310 A 200 200 0 0 1 520 310 Z" '
        'fill="#eaf2f7" stroke="#9bb7c8" stroke-width="1.5" stroke-dasharray="5 6"/>'
    )
    # These are illustrative directions in a slice, not a statistical sampling plot.
    for angle in (-78, -53, -24, 18, 77):
        figure.line(point, polar(point, 177, angle), "outgoing", width=1.5, arrow=True)
    figure.line(point, polar(point, 190, 48), "outgoing", width=3, arrow=True)
    figure.line(point, (320, 91), "normal", width=3, arrow=True)
    figure.angle(point, 60, 0, 48)
    figure.text(348, 237, "θ")
    figure.text(337, 103, "normal n", "normal")
    figure.text(437, 145, "sampled direction", "outgoing", size=18)
    figure.dot(point)
    figure.text(320, 340, "p", anchor="middle")
    figure.text(66, 340, "tangent plane", "muted", size=18)
    figure.text(320, 385, "Trace outward to find the light arriving along each ray.", size=18, anchor="middle")
    figure.save("sdf-hemisphere-vectors.svg")


def lobe(figure, point, radius, weight, color="outgoing", fill="#eaf2f7"):
    """Fill a polar lobe above `point`; `weight(degrees)` is scaled so its peak reaches `radius`."""
    angles = [a / 2 for a in range(-180, 181)]
    values = [max(weight(a), 0.0) for a in angles]
    peak = max(values)
    points = [polar(point, radius * v / peak, a) for a, v in zip(angles, values)]
    figure.parts.append(
        f'<path d="M {pair(point)} ' + " ".join(f"L {pair(p)}" for p in points) + ' Z" '
        f'fill="{fill}" stroke="{COLORS[color]}" stroke-width="2"/>'
    )


def hemisphere_sampling():
    figure = Diagram(425, "Two ways to choose directions",
        "Two cross-sections of the hemisphere above a surface point. On the left, uniform "
        "sampling has the same density in every direction, drawn as a half disc, and its "
        "sample directions spread down to the tangent plane. On the right, cosine-weighted "
        "sampling has density cos theta over pi, drawn as a disc touching the point, and "
        "its sample directions gather near the normal.")
    figure.text(320, 68, "Cross-sections of the hemisphere above a surface point", "muted", size=18, anchor="middle")
    surface_y = 290
    figure.surface(surface_y)
    count = 8
    # Shapes use one scale: radius is proportional to the probability density.
    scale = 120 * math.pi
    panels = [
        (165, "Uniform", "p = 1 / (2π)", lambda a: 1 / (2 * math.pi),
         lambda u: math.degrees(math.acos(1 - u))),
        (475, "Cosine-weighted", "p = cos θ / π", lambda a: math.cos(math.radians(a)) / math.pi,
         lambda u: math.degrees(math.asin(math.sqrt(u)))),
    ]
    for x, title, label, density, invert in panels:
        point = (x, surface_y)
        figure.text(x, 112, title, anchor="middle")
        peak = max(density(a / 2) for a in range(-180, 181))
        lobe(figure, point, scale * peak, density)
        # Evenly spaced samples of the polar-angle distribution, alternating sides.
        for i in range(count):
            angle = invert((i + 0.5) / count) * (1 if i % 2 else -1)
            figure.line(point, polar(point, 112, angle), "outgoing", width=1.5, arrow=True)
        figure.line(point, (x, surface_y - 150), "normal", width=2, dashed=True, arrow=True)
        figure.text(x + 8, surface_y - 140, "n", "normal", size=18)
        figure.dot(point)
        figure.text(x, 362, label, "outgoing", size=18, anchor="middle")
    figure.text(320, 405, "Shaded shapes show each density; arrows are evenly spaced samples from it.",
                "muted", size=17, anchor="middle")
    figure.save("sdf-hemisphere-sampling.svg")


def brdf_lobes():
    figure = Diagram(425, "Where reflected light comes from",
        "Two cross-sections through a surface point, with the view direction 45 degrees to "
        "the left of the normal. Each shaded lobe shows how much light arriving from each "
        "direction reaches the viewer, the BRDF times the cosine. The matte Lambertian lobe "
        "is a round disc around the normal. The glossy GGX lobe, with roughness 0.35, is a "
        "narrow lobe around the mirror direction on the right.")
    figure.text(320, 68, "Light from each direction that reaches the viewer (BRDF × cosine)", "muted", size=18, anchor="middle")
    surface_y = 290
    figure.surface(surface_y)
    view_angle = -45
    alpha = 0.35 * 0.35

    def ggx(light_angle):
        # The tutorial's microfacet BRDF in the plane of incidence, times the cosine.
        v = (math.sin(math.radians(view_angle)), math.cos(math.radians(view_angle)))
        l = (math.sin(math.radians(light_angle)), math.cos(math.radians(light_angle)))
        nv, nl = v[1], l[1]
        if nv <= 0 or nl <= 0:
            return 0.0
        h = (v[0] + l[0], v[1] + l[1])
        length = math.hypot(*h)
        nh, vh = h[1] / length, (v[0] * h[0] + v[1] * h[1]) / length
        a2 = alpha * alpha
        d = a2 / (math.pi * ((1 - nh * nh) + a2 * nh * nh) ** 2)
        g1 = lambda c: 2 * c / (c + math.sqrt(a2 + (1 - a2) * c * c))
        f = 0.9 + 0.1 * (1 - vh) ** 5
        return d * f * g1(nv) * g1(nl) / (4 * nv * nl) * nl

    panels = [
        (165, "Matte (Lambertian)", lambda a: math.cos(math.radians(a))),
        (475, "Glossy (GGX, roughness 0.35)", ggx),
    ]
    for x, title, weight in panels:
        point = (x, surface_y)
        figure.text(x, 112, title, anchor="middle")
        lobe(figure, point, 125, weight)
        figure.line(point, polar(point, 140, -view_angle), "guide", width=1.5, dashed=True)
        figure.line(point, polar(point, 120, view_angle), "incoming", width=3, arrow=True)
        figure.text(x - 92, surface_y - 96, "view v", "incoming", size=18, anchor="end")
        figure.text(x + 104, surface_y - 112, "mirror", "muted", size=17)
        figure.dot(point)
    figure.text(320, 362, "A matte surface gathers light from everywhere above it;",
                size=18, anchor="middle")
    figure.text(320, 388, "a glossy one mostly from near the mirror direction.",
                size=18, anchor="middle")
    figure.save("sdf-brdf-lobes.svg")


def orbit():
    figure = Diagram(400, "Orbit coordinates",
        "Two views of a camera orbiting a target point. In the side view, the camera sits "
        "on a dashed circle around the target; pitch is its angle above the horizon and "
        "distance is the circle's radius. In the top view, yaw is the camera's angle "
        "around the vertical axis, measured from its starting direction.")
    figure.text(320, 68, "The camera stays on a sphere around the target and always looks at it.",
                "muted", size=17, anchor="middle")
    radius = 105

    def place_camera(target, lens):
        degrees = math.degrees(math.atan2(target[1] - lens[1], target[0] - lens[0]))
        figure.line(lens, target, "incoming", width=2, arrow=True)
        figure.camera_icon(lens, degrees)

    # Side view: pitch above the horizon and distance to the target.
    target = (175, 250)
    pitch = 35
    figure.text(target[0], 110, "Side view", anchor="middle")
    figure.circle(target, radius, "guide", dashed=True)
    figure.line((target[0] - radius - 25, target[1]), (target[0] + radius + 25, target[1]), "guide", width=1, dashed=True)
    lens = along(target, (-math.cos(math.radians(pitch)), math.sin(math.radians(pitch))), radius)
    place_camera(target, lens)
    figure.parts.append(
        f'<path d="M {pair(along(target, (-1, 0), 48))} A 48 48 0 0 1 '
        f'{pair(along(target, (-math.cos(math.radians(pitch)), math.sin(math.radians(pitch))), 48))}" '
        f'fill="none" stroke="{COLORS["normal"]}" stroke-width="2"/>'
    )
    figure.text(target[0] - 66, target[1] - 18, "pitch", "normal", size=17, anchor="end")
    middle = along(target, (-math.cos(math.radians(pitch)), math.sin(math.radians(pitch))), radius / 2)
    figure.text(middle[0] + 6, middle[1] - 14, "distance", "outgoing", size=17)
    figure.dot(target)
    figure.text(target[0] + 10, target[1] + 24, "target", size=17)
    figure.text(target[0], 384, "Drag up or down: pitch. Scroll: distance.", "muted", size=16, anchor="middle")

    # Top view: yaw around the vertical axis, from the starting direction.
    target = (470, 240)
    yaw = 40
    figure.text(target[0], 110, "Top view", anchor="middle")
    figure.circle(target, radius, "guide", dashed=True)
    figure.line(target, (target[0], target[1] + radius + 20), "guide", width=1, dashed=True)
    lens = (target[0] + radius * math.sin(math.radians(yaw)), target[1] + radius * math.cos(math.radians(yaw)))
    place_camera(target, lens)
    start = (target[0], target[1] + 48)
    end = (target[0] + 48 * math.sin(math.radians(yaw)), target[1] + 48 * math.cos(math.radians(yaw)))
    figure.parts.append(
        f'<path d="M {pair(start)} A 48 48 0 0 0 {pair(end)}" '
        f'fill="none" stroke="{COLORS["normal"]}" stroke-width="2"/>'
    )
    figure.text(target[0] - 12, target[1] + 76, "yaw", "normal", size=17, anchor="end")
    figure.dot(target)
    figure.text(target[0] + 12, target[1] - 10, "target", size=17)
    figure.text(target[0], 384, "Drag left or right: yaw.", "muted", size=16, anchor="middle")
    figure.save("sdf-orbit-camera.svg")


def offset():
    figure = Diagram(355, "Start the ray just outside the surface",
        "The surface point p lies within the hit-tolerance band. A dashed ray beginning at p "
        "would immediately hit the surface again. The new origin p plus epsilon n is moved "
        "outward along the normal. Its solid blue ray has the same direction. The offset "
        "and tolerance band are exaggerated for visibility.")
    point = (215, 265)
    # The drawn offset/tolerance ratio matches the tutorial's 0.004 / 0.001.
    tolerance = 18
    origin = (point[0], point[1] - 4 * tolerance)
    figure.surface(point[1], depth=31)
    figure.parts.append(
        f'<path d="M 48 {point[1] - tolerance} H 592 V {point[1]} H 48 Z" fill="#f5e8cf"/>'
    )
    figure.line((48, point[1] - tolerance), (592, point[1] - tolerance), "incoming", width=1, dashed=True)
    figure.line(point, (455, 149), "error", width=2, dashed=True, arrow=True)
    figure.line(origin, (455, 77), "outgoing", width=3, arrow=True)
    figure.line(point, (215, 82), "normal", width=2, arrow=True)
    figure.line((181, origin[1]), (181, point[1]), "normal", width=1.5)
    for y in (origin[1], point[1]):
        figure.line((176, y), (186, y), "normal", width=1.5)
    figure.dot(point, "error")
    figure.dot(origin, "outgoing")
    figure.text(163, 235, "ε", "normal")
    figure.text(232, 109, "normal n", "normal")
    figure.text(470, 86, "offset ray", "outgoing", size=18)
    figure.text(469, 157, "original ray", "error", size=18)
    figure.text(196, 181, "p + εn", "outgoing", anchor="end")
    figure.text(215, 290, "p", "error", anchor="middle")
    figure.text(458, 239, "hit tolerance", "incoming", size=18)
    figure.text(66, 289, "surface", "muted", size=18)
    figure.text(320, 331, "Offset and tolerance exaggerated; ray direction unchanged.", size=18, anchor="middle")
    figure.save("sdf-ray-offset.svg")


def microfacets():
    figure = Diagram(310, "Roughness spreads the facet normals",
        "Two magnified surface profiles have the same average horizontal orientation. "
        "The smoother profile has shallow slopes and nearly aligned normals. The rougher "
        "profile has steeper slopes and more widely tilted normals. These are illustrative "
        "profiles, not actual geometry added to the sphere.")
    for start, amplitude, title in [(48, 4, "Smoother"), (352, 22, "Rougher")]:
        figure.text(start + 120, 88, title, anchor="middle")
        points = [(start + i * 40, 202 + (amplitude if i % 2 else -amplitude))
                  for i in range(7)]
        figure.line((start, 202), (start + 240, 202), dashed=True)
        for a, b in zip(points, points[1:]):
            figure.line(a, b, "muted", width=3)
            middle = ((a[0] + b[0]) / 2, (a[1] + b[1]) / 2)
            # In world space the tangent has y = -(b.y-a.y); rotate it toward +y.
            nx, ny = b[1] - a[1], b[0] - a[0]
            length = math.hypot(nx, ny)
            figure.line(middle, along(middle, (nx / length, ny / length), 60),
                        "normal", width=2, arrow=True)
    figure.text(320, 267, "Tiny mirror patches, shown greatly enlarged.", size=18, anchor="middle")
    figure.text(320, 296, "The sphere's visible outline stays the same.", "muted", size=18, anchor="middle")
    figure.save("sdf-microfacets.svg")


def half_vector():
    figure = Diagram(415, "The half-vector is a facet normal",
        "The surface normal n points upward. The view v points up and left and light l "
        "points up and right, both away from the surface point p. The half-vector h is "
        "tilted 20 degrees from n and bisects the 110-degree angle between v and l. "
        "A small mirror facet is perpendicular to h.")
    point = (286, 299)
    figure.surface(point[1], depth=34)
    figure.line(point, polar(point, 207, 0), "normal", width=2, dashed=True, arrow=True)
    figure.line(point, polar(point, 220, -35), "incoming", width=3, arrow=True)
    figure.line(point, polar(point, 243, 75), "outgoing", width=3, arrow=True)
    figure.line(point, polar(point, 213, 20), "error", width=3, arrow=True)
    figure.line(polar(point, 54, -70), polar(point, 54, 110), "ink", width=5)
    figure.angle(point, 70, -35, 20)
    figure.angle(point, 70, 20, 75)
    # Both arcs are the angle between h and one ray; theta elsewhere means the angle from n.
    figure.text(263, 211, "β", size=18)
    figure.text(351, 235, "β", size=18)
    figure.text(132, 101, "view v", "incoming", anchor="middle")
    figure.text(511, 214, "light l", "outgoing", anchor="middle")
    figure.text(271, 89, "n", "normal", anchor="middle")
    figure.text(375, 96, "half-vector h", "error", size=18)
    figure.text(129, 284, "facet", size=18)
    figure.line((179, 279), (231, 280), "muted", width=1)
    figure.dot(point)
    figure.text(286, 328, "p", anchor="middle")
    figure.text(320, 374, "h = normalize(v + l)", size=20, anchor="middle")
    figure.text(320, 402, "Both v and l point away from the surface.", "muted", size=18, anchor="middle")
    figure.save("sdf-half-vector.svg")


if __name__ == "__main__":
    signed_distance()
    sphere_tracing()
    camera()
    normal_light()
    smooth_union()
    mirror()
    reflection_path()
    hemisphere()
    hemisphere_sampling()
    brdf_lobes()
    orbit()
    offset()
    microfacets()
    half_vector()
