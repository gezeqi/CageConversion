#include <Eigen/Core>
#include <iostream>
#include <map>
#include <unordered_set>

#include "igl/readOBJ.h"
#include "igl/writeOBJ.h"
#include <vector>
using namespace Eigen;


struct PointHash {
    size_t operator()(const VectorXd &v) const {
        return std::hash<double>{}(v.norm());
    }
};

struct PointEqual {
    bool operator()(const VectorXd &a, const VectorXd &b) const {
        return (a - b).norm() < 1e-6;
    }
};

int main() {
    // std::string model_name = "beast";
    // std::string model_name = "bench";
    // std::string model_name = "elephant";
    // std::string model_name = "giraffe";
    // std::string model_name = "monkeyhead";
    // std::string model_name = "octopus";
    // std::string model_name = "seahorse";
    // std::string model_name = "shark";
    // std::string model_name = "shark_scale";
    std::string model_name = "data";

    MatrixXd cage_vs;
    MatrixXi cage_fs;
    igl::readOBJ("./models/" + model_name + "/cage.obj", cage_vs, cage_fs);

    // For collecting original vertices.
    std::vector<std::vector<VectorXd> > all_cage_fvs;
    std::vector<VectorXd> cur_face_vs;

    // For collecting deformed vertices.
    std::vector<VectorXd> all_cage_vs;

    for (int f_id{0}; f_id < cage_fs.rows(); ++f_id) {
        VectorXi cur_fv_ids = cage_fs.row(f_id);
        RowVectorXd p003 = cage_vs.row(cur_fv_ids(0));
        RowVectorXd p030 = cage_vs.row(cur_fv_ids(1));
        RowVectorXd p300 = cage_vs.row(cur_fv_ids(2));

        for (int i = 3; i >= 0; --i) {
            for (int j = 0; j < 4 - i; ++j) {
                RowVectorXd cur_p = (i * p003 + j * p300 + (3 - i - j) * p030) / 3;
                // For collecting original vertices.
                cur_face_vs.emplace_back(cur_p);

                // For collecting deformed vertices.
                all_cage_vs.emplace_back(cur_p);
            }
        }

        // For collecting original vertices.
        all_cage_fvs.emplace_back(cur_face_vs);
        cur_face_vs.clear();
    }

    /* ===================== Collect original vertices ===================== */
    std::ofstream file_origin("cps-origin.txt");
    file_origin << std::fixed << std::setprecision(10);
    for (std::vector<VectorXd> &cur_fvs: all_cage_fvs) {
        for (VectorXd &cur_fv: cur_fvs) {
            file_origin << cur_fv.transpose() << " ";
        }
        file_origin << "\n";
    }
    file_origin.close();


    /* ===================== Collect deformed vertices ===================== */
    std::unordered_set<VectorXd, PointHash, PointEqual> unique_vs_set;
    for (const auto &vv: all_cage_vs) {
        unique_vs_set.insert(vv);
    }
    std::vector<VectorXd> unique_cvs;
    unique_cvs.assign(unique_vs_set.begin(), unique_vs_set.end());

    // Construct maps
    std::map<int, int> all_to_unique_map;
    for (int c_id = 0; c_id < all_cage_vs.size(); ++c_id) {
        auto it = std::find_if(unique_cvs.begin(), unique_cvs.end(), [&](const VectorXd &uni_v) {
            return (all_cage_vs.at(c_id) - uni_v).norm() < 1e-6;
        });
        all_to_unique_map[c_id] = std::distance(unique_cvs.begin(), it);
    }

    std::map<int, std::vector<int> > unique_to_all_map;
    for (int c_id = 0; c_id < all_cage_vs.size(); ++c_id) {
        int uni_idx = all_to_unique_map[c_id];
        unique_to_all_map[c_id].emplace_back(uni_idx);
    }

    // std::ofstream file_unique("cps_unique.obj");
    // file_unique << std::fixed << std::setprecision(6);
    // for (VectorXd &cur_v: unique_cvs) {
    //     file_unique << "v " << cur_v.transpose() << "\n";
    // }
    // file_unique.close();


    // Read original vertices
    MatrixXd vs_origin;
    MatrixXi fs_origin;
    igl::readOBJ("./models/" + model_name + "/control_points.obj", vs_origin, fs_origin);

    std::vector<VectorXd> origin_vs;
    for (int c_id = 0; c_id < vs_origin.rows(); ++c_id) {
        origin_vs.emplace_back(vs_origin.row(c_id));
    }

    // Construct some connections between vs_original and unique_cvs
    std::vector<int> original_to_unique;
    for (auto & origin_v : origin_vs) {
        auto it = std::find_if(unique_cvs.begin(), unique_cvs.end(), [&](const VectorXd &uni_v) {
            return (origin_v - uni_v).norm() < 1e-3;
        });
        original_to_unique.emplace_back(std::distance(unique_cvs.begin(), it));
    }

    std::map<int, int> unique_to_origin;
    for (int c_id = 0; c_id < unique_cvs.size(); ++c_id) {
        unique_to_origin[original_to_unique.at(c_id)] = c_id;
    }

    // // Test the vertices are coincides with control_points.obj
    // std::vector<VectorXd> test_vecs;
    // for (int c_id = 0; c_id < unique_cvs.size(); ++c_id) {
    //     test_vecs.emplace_back(unique_cvs.at(original_to_unique[c_id]));
    // }
    // // Store vertices
    // std::ofstream file_test("cps_test.obj");
    // file_test << std::fixed << std::setprecision(6);
    // for (VectorXd &cur_v: test_vecs) {
    //     file_test << "v " << cur_v.transpose() << "\n";
    // }
    // file_test.close();


    // ======================================================
    // Read deformed vertices
    MatrixXd vs_deformed;
    MatrixXi fs_deformed;
    igl::readOBJ("./models/" + model_name + "/deformed_control_points.obj", vs_deformed, fs_deformed);


    std::vector<VectorXd> deformed_vs;
    for (int i = 0; i < vs_deformed.rows(); ++i) {
        deformed_vs.emplace_back(vs_deformed.row(i));
    }

    // Replace the position in all_vs.
    for (int i = 0; i < all_cage_vs.size(); ++i) {
        all_cage_vs[i] = deformed_vs[unique_to_origin[all_to_unique_map[i]]];
    }

    std::ofstream file_deformed("cps-deformed.txt");
    file_deformed << std::fixed << std::setprecision(10);
    int count = -1;
    for (VectorXd &vvv: all_cage_vs) {
        count++;
        if (count >= 10) {
            file_deformed << "\n";
            count = 0;
        }
        file_deformed << vvv.transpose() << " ";
    }
    file_deformed.close();

    return EXIT_SUCCESS;
}
