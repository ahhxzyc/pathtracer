//#include "OctTree.h"
//#include <algorithm>
//#include <iostream>
//#include <vector>
//
//using namespace std;
//
////
////OctTree::OctTree(const vector<Triangle> &tris) {
////    Bound3f aabb;
////    for (const Triangle &tri : tris)
////        aabb = aabb.merge(Bound3f(tri));
////    root = build(aabb, tris);
////    cout << m_cnt << " nodes in the tree.\n";
////    cout << "depth: " << get_depth(root) << endl;
////}
//
//
//TreeNode *OctTree::build(Bound3f bounding, vector<Triangle> tris) {
//    // if (tris.empty())
//    //     return nullptr;
//    // TreeNode *node = new TreeNode;
//    // if (tris.size() == 1) {
//    //     node->tri = tris[0];
//    //     node->aabb = AABB(tris[0]);
//    //     m_cnt ++ ;
//    //     return node;
//    // }
//    // // sort triangles
//    // int idx = longest_axis(tris);
//    // sort(tris.begin(), tris.end(), 
//    //         [idx](const Triangle &a, const Triangle &b) {
//    //             return a.center()[idx] < b.center()[idx];
//    //         }
//    // );
//    // // split and recurse
//    // auto it = tris.begin() + tris.size() / 2;
//    // node->left = build(vector<Triangle>(tris.begin(), it));
//    // node->right = build(vector<Triangle>(it, tris.end()));
//    // node->aabb = node->left->aabb.merge(node->right->aabb);
//    // return node;
//
//    if (tris.empty())
//        return nullptr;
//    TreeNode *node = new TreeNode;
//    node->aabb = bounding;
//    if (tris.size() < 5) {
//        node->tris = tris;
//        m_cnt ++ ;
//        return node;
//    }
//    vector<int> taken(tris.size(), false);
//    for (int i = 0; i < 8; i ++ ) {
//        vector<Triangle> sub_tris;
//        Bound3f sub = bounding.getSubBox(i);
//        for (int j = 0; j < tris.size(); j ++ ) {
//            if (taken[j])
//                continue;
//            if (sub.contain(tris[j])) {
//                sub_tris.push_back(tris[j]);
//                taken[j] = true;
//            }
//        }
//        node->children[i] = build(sub, sub_tris);
//    }
//    for (int i = 0; i < tris.size(); i ++ ) {
//        if (!taken[i]) 
//            node->tris.push_back(tris[i]);
//    }
//    return node;
//}
//
//
//
//void OctTree::intersect(Ray ray, Intersection &inter, int &cnt, float tmin, float tmax) const
//{
//    root->intersect(ray, inter, cnt, tmin, tmax);
//}
//
//
//void TreeNode::intersect(Ray ray, Intersection &inter, int &cnt, float tmin, float tmax) const
//{
//    // // leaf node
//    // if (!left && !right) {
//    //     tri.intersect(ray, inter);
//    //     cnt ++ ;
//    //     return;
//    // }
//    // // inner node
//    // aabb.intersect(ray, inter);
//    // if (!inter.yes)
//    //     return;
//    // Intersection inter_l, inter_r;
//    // left->intersect(ray, inter_l, cnt);
//    // right->intersect(ray, inter_r, cnt);
//    // if (inter_l.yes) {
//    //     if (!inter_r.yes)       inter = inter_l;
//    //     else                    inter = inter_l.t < inter_r.t ? inter_l : inter_r;
//    // } else {
//    //     if (inter_r.yes)        inter = inter_r;
//    //     else                    inter.yes = false;
//    // }
//    if (!aabb.intersect(ray, tmin, tmax)) {
//        return ;
//    }
//    Intersection ret_is;
//    // triangles owned by the node
//    for (const Triangle &tri : tris) {
//        Intersection tis;
//        tri.intersect(ray, tis, tmin, tmax);
//        cnt ++ ;
//        if (tis.yes)
//        {
//            tmax = tis.t;
//            ret_is = tis;
//        }
//    }
//    // triangles in the children
//    for (int i = 0; i < 8; i ++ ) {
//        if (!children[i])
//            continue;
//        Intersection tis;
//        children[i]->intersect(ray, tis, cnt, tmin, tmax);
//        if (tis.yes)
//        {
//            tmax = tis.t;
//            ret_is = tis;
//        }
//    }
//    inter = ret_is;
//}
//
//
//int OctTree::get_depth(TreeNode *r) {
//    if (!r)
//        return 0;
//    int d = 0;
//    for (int i = 0; i < 8; i ++)
//        d = max(d, get_depth(r->children[i]));
//    return d + 1;
//}
//
//
//// int OctTree::longest_axis(const vector<Triangle> &tris) {
////     AABB aabb;
////     for (const Triangle &tri : tris)
////         aabb.merge(AABB(tri));
////     Vec3f d = aabb.m_max - aabb.m_min;
////     if (d[0] > d[1] && d[0] > d[2]) {
////         return 0;
////     } else if (d[1] > d[2]) {
////         return 1;
////     } else {
////         return 2;
////     }
//// }