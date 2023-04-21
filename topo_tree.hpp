/*                         ����������                               *
*                                                                   *
*                   ���˴�ͼ��Ϊ��׼������ʹ��                      *
*                                                                   *
*                   �κ�ͼ״�ṹ������ת��Ϊ��״�ṹ                *
*                   ����ĳ�ֹ�����������Եõ���ͬ�����ṹ          *
*                   ���磺e{e1,e2{ee1,ee2},e3}                      *
*                   һ����״������ʽ��Ϊ��                          *
*                       shape = {topo, coord, space}                *
*                                                                   *
*                   ���⣺ͼ��Ӧ�˷��� ����Ӧ�˷��̵Ľ�ṹ         *
*                                                                   *    
* * * * * * * * * ** * * * * *  ʾ������ * * * * * * * * * ** * * * * 

        topoE te =  {"te",{0}, false };
        topoE te1 = { "te1",{0, 1}, false};
        topoE te2 = { "te2",{2, 3}, false};
        topoE te3 = { "te3", {0}, true };
        topoE te4 = { "te4",{1, 2}, false };
        topoE te5 = { "te5",{1, 3}, false };
      
        te.children.emplace_back(&te1);
        te.children.emplace_back(&te3);
        te3.children.emplace_back(&te4);
        te3.children.emplace_back(&te5);
        te.children.push_back(&te2);

        static const vec3 p_set[] = {
            vec3(-1, -1, 1) ,vec3(0.5, 1, 1), 
            vec3(-2, 1.8, 3), vec3(2, 1.8, -3)
        };
        {
            VECLIST e;
            vec3 p0;
            // ����������
            te.walkL([&e, &p0](topoE* to, real t) {
                coord3 c1(p_set[to->e.start]), c2(p_set[to->e.end]);
                c1.rot(-45, vec3::UZ); c2.rot(45, vec3::UX);
                vec3 p = blender::slerp(c1, c2, t).o;

                if (VECLIST_ADD(e, p)) {
                    if (p0 != vec3::ZERO)
                        ptr(p0, p, 0.01);
                    p0 = p;
                }
            });
            PRINT(e.size() << " = " << COMBINE_STEPS(5,3));
        }
*/
namespace TOPO_TREE 
{	
	struct tedge;
	vector<tedge> tedges;					        // ���б�
    struct tshape;
    vector<tshape> tshapes;                         // ���б�

	// ------------------------------
	// ��
	// ------------------------------
    struct tedge
    {
        string name;
        struct edge { int start, end; } e;          // �߶���(Ҷ�ڵ���)
        bool bmutex = false;                        // ���б��Ƿ񻥳� 

        // ������ͬ��Լ��������ռ�
        Lineiter space = { 0, 1, 5 };
        int cs = 0;

        tedge* parent = 0;                      // ��
        std::vector<tedge*> children;           // ���б�

		tedge() {}

		// ��ʼ���б��캯��
		tedge(string name_, edge e_, bool bmutex_ = false)
			: name(std::move(name_)), e(std::move(e_)), bmutex(bmutex_)
		{}
        tedge(edge e_, bool bmutex_ = false)
            : e(std::move(e_)), bmutex(bmutex_)
        {}
		// �ƶ����캯��
		tedge(tedge&& other) noexcept
			: name(std::move(other.name)),
			e(std::move(other.e)),
			bmutex(other.bmutex),
			space(std::move(other.space)),
			parent(other.parent),
			children(std::move(other.children))
		{
			other.parent = nullptr;
		}
		// �ƶ���ֵ����
		tedge& operator=(tedge&& other) noexcept {
			if (this != &other) {
				name = std::move(other.name);
				e = std::move(other.e);
				bmutex = other.bmutex;
				space = std::move(other.space);
				parent = other.parent;
				children = std::move(other.children);
				other.parent = nullptr;
			}
			return *this;
		}
        bool is_compound()
        {
            return !children.empty();
        }
        void operator += (tedge* et)
        {
            children.push_back(et);
        }

        void walkL(std::function<void(tedge* to, real t)> cb)
        {
            //PRINT("walkdownL " << name << "," << this->e.start << "," << this->e.end);
            if (children.empty()) {
                if (cb)
                {
                   // PRINT("[" << space.min << "," << space.max << "," << space.steps << "]");
                    space.walk([this, &cb](int i, real t) { cb(this, t); });
                }
                return; // ����
            }
            if (bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
                //PRINT("����ѡ LEFT");
                children.front()->walkL(cb);

            } else {
                for (auto c : children)
                    c->walkL(cb);
            }
        }
        void walkR(std::function<void(tedge* to, real t)> cb)
        {
           // PRINT("walkdownR " << name << "," << this->e.start << "," << this->e.end);
            if (children.empty()) {
                if (cb)
                {
                    PRINT("[" << space.min << "," << space.max << "," << space.steps << "]");
                    space.walk([this, &cb](int i, real t) { cb(this, t); });
                }
                return; // ����
            }
            if (bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
               // PRINT("����ѡ RIGHT");
                children.back()->walkR(cb);

            } else {
                for (auto c : children)
                    c->walkR(cb);
            }
        }
        static void walkL(tedge* to, std::function<void(tedge* to, int, int)> cb)
        {
           // PRINT("walkdownL " << to->name);
            if (to->children.empty()) {
                if(cb)
                    cb(to, to->e.start, to->e.end);
                return; // ����
            }
            if (to->bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
                
              //  PRINT("����ѡ LEFT");
                walkL(to->children.front(), cb);
                
            } else {
                for (auto c : to->children)
                    walkL(c, cb);
            }
        }
        static void walkR(tedge* to, std::function<void(tedge* to, int, int)> cb)
        {
            //PRINT("walkdownR " << to->name);
            if (to->children.empty()) {
                if (cb)
                    cb(to, to->e.end, to->e.start);
                return; // ����
            }
            if (to->bmutex) // ���Ը���CHILDREN�Ƿ����1�����ж�
            {
               // PRINT("����ѡ RIGHT");
                walkR(to->children.back(), cb);

            } else {
                for (auto it = to->children.rbegin(); it != to->children.rend(); ++it) {
                    walkR(*it, cb);
                }
            }
        }
        void clear(tedge* to)
        {
            for (auto c : to->children) {
                clear(c);
            }
            delete to;
        }
        void clear()
        {
            for (auto c : children)
                clear(c);
        }
    };
	// ------------------------------
	// ����
	// ------------------------------
	struct tsurface
	{
		std::vector<tedge*> tedges;				// ��ʱ�����б�ʽ���˽ṹ
	};
	// ------------------------------
	// ��״
	// ------------------------------
	struct tshape
	{
		std::vector<tsurface*> tsurfaces;	    // �������б�
	};
}