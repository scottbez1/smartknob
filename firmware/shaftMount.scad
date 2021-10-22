$fn=30;


module ic() {
    linear_extrude(height=2) {
        difference() {
            square([7.75, 10.5], center=true);
            circle(d=4.2);
        }
    }
    translate([0, 0, 2]) {
        linear_extrude(height=2) {
            difference() {
                square([7.75, 10.5], center=true);
                square([5.25, 7], center=true);
            }
        }
    }
}

module mountHoles(d=3.2) {
    translate([19/2, 0]) {
        circle(d=d);
    }
    translate([-19/2, 0]) {
        circle(d=d);
    }
    translate([0, 19/2]) {
        circle(d=d);
    }
    translate([0, -19/2]) {
        circle(d=d);
    }
}

module base() {
    linear_extrude(height=2) {
        difference() {
            circle(d=24);
            circle(d=3.4);
            mountHoles();
        }
    }
    linear_extrude(height=3) {
        difference() {
            mountHoles(4);
            mountHoles();
        }
    }
    linear_extrude(height=22) {
        difference() {
            circle(d=4.2);
            circle(d=3.4);
        }
    }
}

base();
translate([20, 0]) {
    ic();
}