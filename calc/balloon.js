/*
 * CONSTANTS
 *
 * Everything is in SI units.
 */

// gas constant
const R = 8.3145;
const R_sp = 287.058;

// earth gravity
const m_earth = 5.97e24;
const r_earth = 6.371e6;
const G = 6.674e-11;

// drag constants
const C_D = 0.3 // sphere coeff. of drag

// balloon characteristics
const m_empty = 0.8; // kg
const r_burst = 3.505;
const m_payload = 0.9; // kg
const beta = 0.02; // extra pressure on inside of balloon due to stretchiness of latex

// gas molar masses
const M_atm = 0.02896;
const M_he = 0.00401;

// helium density @ STP
const rho_he_STP = 0.1786;

// base constants at various atmospheric levels
const rho_atm = [1.225, 0.36391, 0.08803, 0.01322, 0.00143, 0.00086, 0.000064];	// mass density
const T_atm = [288.15, 216.65, 216.65, 228.85, 270.65, 270.65, 214.65];			// temperature
const L_atm = [-0.0065, 0.0, 0.0001, 0.0028, 0.0, -0.0028, -0.002];				// lapse rate
const h_atm = [0, 11e3, 20e3, 32e3, 47e3, 51e3, 51e3, 71e3];					// height
const p_atm = [101325, 22632.1, 5474.89, 868.02, 110.91, 66.94, 3.96];			// pressure

// simulation constants
const h_step = 5.0;
const base_h = 0;

/*
 * Main routine, program entry point
 */
function main() {
	for (var V = 0.5; V < 2.5; V += 0.1) {
		var string =  "";
		string += `V = ${V.toFixed(4)},  \t`;
        string += `v = ${getVelocity(V, base_h).toFixed(3)},  \t`;
        string += `h = ${getBurstHeight(V).toFixed(0)} m,  \t`;
        string += `t = ${(getAscentTime(V) / 60).toFixed(0)} min`;

        println(string);
	}
}

/*
 * Calculate balloon burst height
 */
function getBurstHeight(V_0) {
	var isDone = false;
	var h = base_h;

	var n = getMoles(V_0);

	while (!isDone) {
		if (getRadius(getVolume(n, h)) > r_burst) {
			isDone = true;
		} else if (getVelocity(V_0, h) <= 0) {
			isDone = true; // coming down now
		}

		h += h_step;
	}

	return h - h_step;
}

/*
 * Calculate time to reach burst height
 */
function getAscentTime(V_0) {
	var t = 0;
	
	var h = getBurstHeight(V_0);

	for (var y = base_h; y < h; y += h_step) {
		t += h_step / getVelocity(V_0, y);
	}

	return t;
}

/*
 * Returns velocity at height. Positive is up.
 */
function getVelocity(V_0, h) {
	var n = getMoles(V_0);
	var V = getVolume(n, h);
	var g = getGravitationalAcceleration(h);

	var gravitationalForce = -g * getTotalMass(n);

	var buoyancyForce = getDensity(h) * V * g;

	return getTerminalVelocity(n, h, buoyancyForce + gravitationalForce);
}

/*
 * Calculate terminal velocity at a certain height, given net forces
 */
function getTerminalVelocity(n, h, netForce) {
	var rho = getDensity(h);
	var V = getVolume(n, h);
	var A = getCrossSectionalArea(V);

	return Math.sign(netForce) * Math.sqrt((2 * Math.abs(netForce)) / (rho * A * C_D));
}

/*
 * Calculate moles of helium required for certain ground volume
 */
function getMoles(V_he) {
	return getPressure(0) * V_he / (R * getTemperature(0));
}

/*
 * Return mass of helium given moles
 */
function getMass(n) {
	return n * M_he;
}

/*
 * Calculate volume at a height given moles of helium
 */
function getVolume(n, h) {
	return n * R * getTemperature(h) / (getPressure(h) * (1 + beta));
}

/*
 * Calculate atmospheric pressure at a certain height
 */
function getPressure(h) {
	var b = getLayerIndex(h);

	var p = p_atm[b];
	var T = T_atm[b];
	var L = L_atm[b];
	var h_b = h_atm[b];
	var g = getGravitationalAcceleration(h);

	if (L === 0) { // no temp variation
		return p * Math.exp(-g * M_atm * (h - h_b) / (R * T));
	} else { // temp variation
		return p * Math.pow(T / (T + L*(h - h_b)), (g * M_atm) / (R * L));
	}
}

/*
 * Calculate air density at a certain height
 */
function getDensity(h) {
	var b = getLayerIndex(h); // layer of atmosphere
	
	var rho = rho_atm[b];
	var T = T_atm[b];
	var L = L_atm[b];
	var h_b = h_atm[b];
	var g = getGravitationalAcceleration(h);

	var x = 1 - L*(h - h_b) / T;
	var y = 1 + (g * M_atm) / (R * L);
	var z = Math.pow(x, y);

	if (L === 0) { // no temp variation
		return rho * Math.exp(-g * M_atm * (h - h_b) / (R * T));
	} else { // temp variation
		return rho * Math.pow(1 - L*(h - h_b) / T, 1 + (g * M_atm) / (R * L));
	}
}

/*
 * Calculate air temperature at a certain height
 */
function getTemperature(h) {
	var b = getLayerIndex(h);

	var T = T_atm[b];
	var L = L_atm[b];
	var h_b = h_atm[b];

	return T + L*(h - h_b);
}

/*
 * Find layer of the atmosphere a certain height is at.
 */
function getLayerIndex(h) {
	if (h >= 0 && h < 11e3) {
		return 0;
	} else if (h >= 11e3 && h < 20e3) {
		return 1;
	} else if (h >= 20e3 && h < 32e3) {
		return 2;
	} else if (h >= 32e3 && h < 47e3) {
		return 3;
	} else if (h >= 47e3 && h < 51e3) {
		return 4;
	} else if (h >= 51e3 && h < 71e3) {
		return 5;
	} else if (h >= 71e3) {
		return 6;
	} else {
		throw new Error("height must be a positive number");
	}
}

/*
 * Calculate g for a certain height
 */
function getGravitationalAcceleration(h) {
	return G * m_earth / Math.pow(r_earth + h, 2);
}

/*
 * Calculate total probe mass
 */
function getTotalMass(V_he) {
	return m_empty + m_payload + getMass(V_he);
}

/*
 * Calculate cross-sectional area of probe
 */
function getCrossSectionalArea(V_he) {
	var radius = getRadius(V_he);
	return Math.PI * radius * radius;
}

/* 
 * Calculate radius of balloon
 */
function getRadius(V_he) {
	return Math.pow(V_he * 0.75 / Math.PI, 1/3);
}

/*
 * Print-out functions
 */
function println(txt) {
	txt = txt || "";
	$("#output").innerHTML += txt + "<br />";
}

// if in node JS, 
if (typeof window === "undefined") {
    println = console.log.bind(console, "");

    main();
}
