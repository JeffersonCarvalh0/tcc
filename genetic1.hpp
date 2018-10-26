# ifndef GENETIC1_HPP
# define GENETIC1_HPP

# include <vector>
# include <list>
# include <set>
# include <map>
# include <random>

/* Tuples will be assigned to periods */
struct Tuple {
    int label, teacher, subject, grade;

    Tuple(int label = -1, int teacher = -1, int grade = -1, int subject = -1):
        label(label), teacher(teacher), subject(subject), grade(grade) {}
};

/* This funciton will create the actual tuples */
std::vector<Tuple> createTuples(int teacher, std::set<int> &chosen_subjects, std::map<int, int> &subjects, std::vector<int> &workloads) {
    std::vector<Tuple> ans;
    int total = 0;
    for (auto &e : chosen_subjects) total += workloads[e];
    ans.reserve(total);

    int cur_label = 0;
    for (auto &e : chosen_subjects) {
        for (int i = 0; i < workloads[e]; ++i)
            ans.push_back(Tuple(cur_label, teacher, e, subjects[e])), ++cur_label;
    }

    return ans;
}

/*
    The first Genetic Algorithm. This will run before the main algorithm from the
    article. Its purpose is the give a set of optimal timetables for a Teacher.
    This must be done for every Teacher in order to have a initial set of timetables
    for the main Genetic Algorithm.
*/
class GA1 {
public:
    int pop_size, periods_size, periods_per_day, max_gen;
    std::vector<std::vector<Tuple>> population;
    std::vector<Tuple> tuples;
    std::vector<int> fitnesses, workloads;
    std::vector<bool> out_periods, occupied_periods;

    std::random_device rd;
    std::default_random_engine generator;
    std::uniform_int_distribution<int> random_period;

    GA1(std::vector<Tuple> tuples, std::vector<int> workloads, std::vector<bool> out_periods, int pop_size = 10, int periods_size = 30, int periods_per_day = 6, int max_gen = 500):
    tuples(tuples), workloads(workloads), out_periods(out_periods), pop_size(pop_size), periods_size(periods_size), periods_per_day(periods_per_day), max_gen(max_gen) {

        // Initializing the attributes
        generator = std::default_random_engine(rd());
        random_period = std::uniform_int_distribution<int>(0, periods_size);
        population = std::vector<std::vector<Tuple>>(pop_size, std::vector<Tuple>(periods_size));
        fitnesses.resize(pop_size);

        // Creating the first generation
        for (int i = 0; i < pop_size; ++i) {
            occupied_periods = out_periods;
            for (int j = 0; j < int(tuples.size()); ++j) {
                int p;
                do p = random_period(generator); while (occupied_periods[p]);
                population[i][p] = tuples[j];
                occupied_periods[p] = true;
            }
        }

        // Calculates the fitness from each chromossome
        for (int i = 0; i < pop_size; ++i) fitnesses[i] = fitness(i);
    }

    /* Calculates the finess of a chromossome(solution) */
    int fitness(int ch) {
        int cost = 0;

        // Checking for classes in the same day for the same grade
        for (int i = 0; i < periods_size / periods_per_day; ++i) {
            std::map<int, int> m;
            for (int j = i * periods_per_day; j < (i + 1) * periods_per_day; ++j) {
                if (m.find(population[ch][j].subject) == m.end())
                    m.insert({ population[ch][j].subject, 0 });
                else
                    ++m[population[ch][j].subject];
            }
            for (auto &e : m) if (e.second > 1) cost += 10;
        }

        // Checking for windows in the timetable
        for (int i = 0; i < periods_size / periods_per_day; ++i) {
            for (int j = i * periods_per_day; j < (i + 1) * periods_per_day; ++j) {
                if (j > i * periods_per_day && j < i + 1 &&
                    population[ch][j - 1].label != -1 &&
                    population[ch][j + 1].label != -1 &&
                    population[ch][j].label == -1) cost += 10;
            }
        }

        return 100 - cost;
    }

    /*
        Probabilistically selects the parents based on their fitnesses
        Roulette Wheel method
    */
    void select(int &parent1, int &parent2) {
        int total = 0;
        for (int i = 0; i < pop_size; ++i) total += fitnesses[i];

        std::uniform_int_distribution<int> v(0, total);
        int chosen = v(generator);

        for (int i = 0; chosen > 0; ++i) {
            chosen -= fitnesses[i];
            if (chosen <= 0) { parent1 = i; break; }
        }

        chosen = v(generator);
        for (int i = 0; chosen > 0; ++i) {
            chosen -= fitnesses[i];
            if (chosen <= 0) { parent2 = i; break; }
        }
    }

    /* Checks for hard constraints violated and fix them */
    void fix(std::vector<Tuple> &child) {
        // Checking for missing and repeated tuples
        std::vector<int> count(tuples.size());
        std::map<int, int> repeated;
        std::set<int> missing;

        for (int i = 0; i < int(child.size()); ++i) {
            if (child[i].label != -1) {
                ++count[child[i].label];
                if (count[child[i].label] > 1) repeated.insert({ i, child[i].label });
            }
        }

        for (int i = 0; i < int(count.size()); ++i)
            if (count[i] == 0) missing.insert(i);

        for (auto &e : repeated) {
            int cur_missing = *missing.begin();
            child[e.first] = tuples[cur_missing];
            --count[e.second]; missing.erase(cur_missing);
        }
    }

    /* Creates two childs from a pair of parents */
    void crossover(int parent1, int parent2, std::vector<Tuple> &child1, std::vector<Tuple> &child2) {
        int pivot = random_period(generator);

        for (int i = 0; i < pivot; ++i) {
            child1[i] = population[parent1][i];
            child2[i] = population[parent2][i];
        }

        for (int i = pivot; i < periods_size; ++i) {
            child1[i] = population[parent2][i];
            child2[i] = population[parent1][i];
        }

        fix(child1); fix(child2);
    }

    /* Creates a new population */
    void breed() {
        std::vector<std::vector<Tuple>> new_population(pop_size, std::vector<Tuple>(periods_size));
        int survivors_qtd = pop_size - int(pop_size * 0.85);

        // Select the survivors using elitism
        std::multimap<int, int> best;
        for (int i = 0; i < pop_size; ++i) best.insert({ fitnesses[i], i });

        int i = 0;
        for (auto it = best.begin(); it != best.end() && i < survivors_qtd; ++it, ++i)
            new_population[i] = population[it->second];

        // Probabilistically chooses the parents
        int parent1, parent2;
        select(parent1, parent2);

        // Generate the rest of the new population through crossover
        std::vector<Tuple> child1, child2;
        for (int i = survivors_qtd; i < pop_size; ++i) {
            crossover(parent1, parent2, child1, child2);
            new_population[i] = child1;
            if (i + 1 < pop_size) new_population[i += 1] = child2;
        }

        population = new_population;
    }
};

# endif /* end of include guard: GENETIC1_HPP */
