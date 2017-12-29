#include <iostream>
#include <caf/all.hpp>
#include <caf/io/all.hpp>
#include <chrono>
#include <boost/program_options.hpp>

#include "KBInput.h"
#include "Minefield.h"

using namespace caf;
using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    po::options_description od("Allowed options");
    od.add_options()("help", "Show this help")("width,w", po::value<int>(), "Width")("height,h", po::value<int>(),
            "Height")("mines,m", po::value<int>(), "Number of mines");
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(od).run(), vm);
    } catch (const po::error& e) {
        cerr << "Error parsing options: " << e.what() << endl;
        return 2;
    }
    po::notify(vm);
    if (vm.count("help")) {
        cout << od << endl;
        return 1;
    }
    uint8_t width = 10;
    uint8_t height = 10;
    uint8_t mines = 20;
    if (vm.count("width") && vm.count("height")) {
        width = vm["width"].as<int>();
        height = vm["height"].as<int>();
    } else if (vm.count("width") ^ vm.count("height")) {
        cout << "Need to specify both width and height or none." << endl;
        return 1;
    }
    if (vm.count("mines")) {
        mines = vm["mines"].as<int>();
    }
    actor_system_config conf;
    conf.add_error_category(minefield_error_atom::value, [=](uint8_t val, atom_value av, const message&) {
        return to_string(static_cast<minefield_error>(val));
    });
    actor_system system(conf);
    auto field = system.spawn<Minefield>();
    auto input = system.spawn<KBInput>(field);
    scoped_actor self(system);
    self->send(field, init_atom::value, width, height, mines);
    self->send(field, dump_atom::value, false);
    bool exit = false;
    string i;
    while (!exit) {
        cin >> i;
        self->request(input, infinite, kb_input_atom::value, i).receive([&](minefield_result r) {
            if (r != minefield_result::ok) {
                exit = true;
            }
        }, [&](error x) {
            aout(self) << "Error: " << system.render(x) << endl;
        });
    }
    self->send_exit(input, exit_reason::user_shutdown);
    self->send_exit(field, exit_reason::user_shutdown);
}
