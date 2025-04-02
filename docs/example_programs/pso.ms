/// Particle swarm optimization.
/// This algorithm is tested on Ackley's function,
/// which has global minimum at [0:0]

class Particle {
    _W_COEF = 0.5
    _CP_COEF = 2.05
    _CG_COEF = 2.05

    fun Particle(bounds) {
        this.velocity = []
        for (i: 0..bounds.length()) {
            this.velocity += [rand_float(-1, 1)]
        }
        this.position = []
        for(bound: bounds) {
            this.position += [rand_float(bound[0], bound[1])]
        }
        this.best_position = this.position
        this.best_value = nil
    }

    fun evaluate_position(function, fitness_function) {
        value = function(<<this.position)
        if (this.best_value == nil || fitness_function(value, this.best_value)) {
            this.best_value = value
            this.best_position = this.position
        }
    }

    fun update_velocity(best_position) {
        rp = rand_float(0, 1) * this._CP_COEF
        rg = rand_float(0, 1) * this._CG_COEF
        for (i : 0..this.velocity.length()) {
            pos_diff = best_position[i] - this.position[i]
            this.velocity[i] = this._W_COEF * this.velocity[i] + rp * pos_diff + rg * pos_diff
        }
    }

    fun update_position(bounds) {
        for (i : 0..this.position.length()) {
            this.position[i] = this.position[i] + this.velocity[i]
            if (this.position[i] < bounds[i][0]) {
                this.position[i] = bounds[i][0]
            } else if (this.position[i] > bounds[i][1]) {
                this.position[i] = bounds[i][1]
            }
        }
    }

    fun get_best_position() = this.best_position
    fun get_best_value() = this.best_value

    fun __String() {
        return "Pos: " ++ this.position ++ ", Vel: " ++ this.velocity
    }
}

fun pso(function, bounds, fitness_function=fun(a, b) = a < b, particle_amount=15, max_iterations=30) {
    swarm = []
    for (_ : 0..particle_amount) {
        swarm += [Particle(bounds)]
    }
    best_position = []
    for (i : 0..bounds.length()) {
        best_position += [bounds[i][0]]
    }
    best_value = nil
    for (i : 0..max_iterations) {
        for (particle: swarm) {
            ~particle.evaluate_position(function, fitness_function)
            if (best_value == nil || fitness_function(particle.get_best_value(), best_value)) {
                best_value = particle.get_best_value()
                // TODO: Deepcopy
                best_position = [particle.get_best_position()[0], particle.get_best_position()[1]]
            }
        }
        for (particle : swarm) {
            ~particle.update_velocity(best_position)
            ~particle.update_position(bounds)
        }
    }
    return best_position
}

fun ackleys_function(x, y) {
    import ::Math.e, ::Math.pi, ::Math.cos
    return -20*e^(-0.2*(0.5*(x^2+y^2))^0.5)-e^(0.5*(cos(2*pi*x)+cos(2*pi*y)))+e+20
}

pso(ackleys_function, [[-50, 50], [-50, 50]], fun(a, b) = a < b, 20, 100)
"\n"
