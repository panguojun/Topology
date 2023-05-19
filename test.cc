// 三通

void drawTee() {

    vec3 p_set[] = {
         vec3(0.0f, 0.0f, 0),
         vec3(-1.0f, 0.0f, 0),
         vec3(-1.0f, 3.0f, 0),
         vec3(0.0f, 1.0f, -1),
         vec3(0.0f, 1.0f, 0),
         vec3(0.0f, 2.0f, 0),
         vec3(0.0f, 3.0f, -1),
         vec3(1.0f, 0.0f, 0),
         vec3(1.5f, 3.0f, 0)
    };
    // 三通拓扑: te1->te2->te3 管口平行边，其中te2跟一个孔(te22)相连接
    topoE te1 = { "e1",{1, 2} }; te1.space.steps = 13;
    topoE te2 = { "e2" };
    topoE te21 = { "e21",{3,4} };
    topoE te22 = { "e22", true };
    topoE te221 = { "e221",{4,5} };
    topoE te222 = { "e222",{4,5} };
    topoE te23 = { "e23",{5,6} };
    topoE te3 = { "e3",{7, 8} }; te3.space.steps = 13;
    {
        te22.addchild(&te221);
        te22.addchild(&te222);

        te2.addchild(&te21);
        te2.addchild(&te22);
        te2.addchild(&te23);
    }
    VECLIST e1, e21, e22, e2, e3;
    {
        coord3 c(-vec::UY, vec::UX, vec::UZ); c.o = (p_set[1] + p_set[2]) / 2;
        Lineiter({ 0, 2 * PI, 12 + 1 }).walk([&](int j, float phi) {
            vec3 vertex = c.o + (c.ux * cos(phi) + c.uz * sin(phi)) * 0.5;
            e1.push_back(vertex);
            pt3d(vertex);
            });
    }
    {// 孔洞
        TopoLib::topo_edge_holeL(e21, te2, p_set);
        TopoLib::topo_edge_holeR(e22, te2, p_set);
        {
            coord3 c(-vec::UY, vec::UX, vec::UZ); c.o = (p_set[3] + p_set[6]) / 2;
            Lineiter({ 0, 2 * PI, (int)e21.size() }).walk([&](int j, float phi) {
                e21[j].p.y = (vec2(0, 0.5).rotcpy(2 * PI / 12 * (4 - j))).x + 1.5;
                e21[j].p.z = (vec2(0, 0.5).rotcpy(2 * PI / 12 * (4 - j))).y;
                color = 0xFF00FF00;
                pt3d(e21[j].p, 0.1);


                e22[j].p.y = (vec2(0, 0.5).rotcpy(2 * PI / 12 * (4 - j))).x + 1.5;
                e22[j].p.z = (vec2(0, 0.5).rotcpy(2 * PI / 12 * (4 - j))).y;
                color = 0xFF0000FF;
                pt3d(e22[j].p, 0.1);
                color = 0xFFFFFFFF;
                });
        }

        //{// 孔洞内边
        //    TopoLib::topo_edge_holeL(e2L, te22, p_set);
        //    e2.insert(e2.end(), e2L.rbegin(), e2L.rend());

        //    TopoLib::topo_edge_holeR(e2R, te22, p_set);
        //    e2.insert(e2.end(), e2R.rbegin(), e2R.rend());
        //}

        for (auto it = e21.begin(); it != e21.end(); it++)
        {
            if ((*it).tag == (int)&te221)
            {
                e2.push_back((*it));
            }

        }
        for (auto it = e22.rbegin() + 1; it != e22.rend(); it++)
        {
            if ((*it).tag == (int)&te222)
            {
                e2.push_back((*it));
            }
            else if (e2.size() == 8)
            {
                e2.push_back((*it));
                pt3d(*it, 0.25);
                break;
            }
        }
        closeedge(e2);
    }
    {
        coord3 c(-vec::UY, vec::UX, vec::UZ); c.o = (p_set[7] + p_set[8]) / 2;
        Lineiter({ 0, 2 * PI, 12 + 1 }).walk([&](int j, float phi) {
            vec3 vertex = c.o + (c.ux * cos(phi) + c.uz * sin(phi)) * 0.5;
            e3.push_back(vertex);
            pt3d(vertex);
            });
    }
    // 绘制
    binvnorm = true;
    face0(e1, e21); face0(e22, e3);
    extrudedgex(e2, 2);
    binvnorm = false;
}