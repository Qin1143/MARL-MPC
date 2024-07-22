#include "mpc_controller/mpc_controller.hpp"
#include "math.h"
#include "iostream"

#define PI 3.1415926
#define T 0.03 // 采样时间
#define w0 1.0
#define w1 0.5
#define Ku 1
#define Kl 1

// 使用Eigen和std命名空间
using namespace Eigen;
using namespace std;
USING_NAMESPACE_QPOASES // 引入QP-OASES命名空间

// 定义MPC控制器类中的MPC_Solve_qp方法
// 输入参数：当前状态X_k，参考状态X_r，参考输入U_r，预测步长N
// 返回值：优化后的控制输入序列U_result
MatrixXd MPC_controller::MPC_Solve_qp(Eigen::Vector3d X_k,std::vector<Eigen::Vector3d >X_r,std::vector<Eigen::Vector2d >U_r,const int N)
{
    //cout<<"current w : "<<w_max<<endl;
    ////根据参考输入计算出的系数矩阵
    vector<MatrixXd> A_r(N),B_r(N),A_multiply1(N); // 状态和控制输入的线性化矩阵
    MatrixXd O_r(3*N,1); // 偏移向量
    MatrixXd A_bar(3*N,3); // 扩展状态转移矩阵
    MatrixXd X_ref(3*N,1); // 参考状态向量
    MatrixXd A_multiply2;
    MatrixXd B_bar = MatrixXd::Zero(3*N,2*N); // 扩展控制输入矩阵
    MatrixXd C_bar = MatrixXd::Identity(3*N,3*N); // 状态转移矩阵的累积
    MatrixXd A_r_init = MatrixXd::Zero(3,3);
    MatrixXd B_r_init = MatrixXd::Zero(3,2);
    MatrixXd eye_3 = MatrixXd::Identity(3,3);
    MatrixXd Q = MatrixXd::Identity(3*N,3*N)*omega0;
    MatrixXd R = MatrixXd::Identity(2*N,2*N)*omega1;
//    cout<<"Ur 1 : "<<endl<<U_r[0]<<endl;
//    cout<<"Xr 1 : "<<endl<<X_r[0]<<endl;
//    cout<<"Xk : "<<endl<<X_k<<endl;
    for(int k=0;k<N;k++) // 线性化状态转移矩阵和控制输入矩阵
    {
        A_r[k] = A_r_init;
        B_r[k] = B_r_init;
        A_r[k](0,2) = -U_r[k](0)*sin(X_r[k](2));
        A_r[k](1,2) = U_r[k](0)*cos(X_r[k](2));
        Vector3d temp_vec = -T*A_r[k]*X_r[k];
        O_r.block<3,1>(k*3,0) = temp_vec;
        A_r[k] = eye_3+T*A_r[k];
        B_r[k](0,0) = cos(X_r[k](2))*T;
        B_r[k](1,0) = sin(X_r[k](2))*T;
        B_r[k](2,1) = T;
        X_ref.block<3,1>(k*3,0) = X_r[k];
//        cout<<"B_r "<<k+1<<" : "<<endl<<B_r[k]<<endl;

        if(k==0) A_multiply1[k] = A_r[k];
        else A_multiply1[k] = A_multiply1[k-1]*A_r[k];
        A_bar.block<3,3>(3*k,0) = A_multiply1[k];
    }
//    cout<<"Ar 1 : "<<endl<<A_r[0]<<endl;
//    cout<<"Br 1 : "<<endl<<B_r[0]<<endl;
//    cout<<"Or 1 : "<<endl<<O_r.block<3,1>(0,0)<<endl;
//    cout<<"O_r : "<<endl<<O_r<<endl;

    for(int k=0;k<N;k++)
    {
        B_bar.block<3,2>(3*k,2*k) = B_r[k];
        A_multiply2 = eye_3;
        for(int i=0;i<k;i++)
        {
            A_multiply2 = A_multiply2*A_r[k-i];
            C_bar.block<3,3>(3*k,3*(k-1-i)) = A_multiply2;
            B_bar.block<3,2>(3*k,2*(k-1-i)) = A_multiply2*B_r[k-1-i];
        }
    }

    //cout<<"C bar : "<<endl<<C_bar<<endl;
    // 构建QP问题的Hesse矩阵和一次项系数
    MatrixXd E =A_bar*X_k+C_bar*O_r-X_ref;
    MatrixXd Hesse = 2*(B_bar.transpose()*Q*B_bar+R);      ////Hesse矩阵
    VectorXd gradient = 2*B_bar.transpose()*Q*E;       ////一次项系数

    // 初始化QP问题的各种参数
    real_t H[2*N*2*N],g[2*N],A[2*N],lb[2*N],ub[2*N],lbA[1],ubA[1];
    lbA[0] = N*(v_min+w_min)/Ku;
    ubA[0] = N*(v_max+w_max)/Kl;
    for(int i=0;i<2*N;i++)
    {
        g[i] = gradient(i);
        A[i] = 1;
        if(i%2==0)
        {
            lb[i] = v_min;
            ub[i] = v_max;
        }
        else
        {
            lb[i] = w_min;
            ub[i] = w_max;
        }
        for(int j=0;j<2*N;j++)
        {
            H[i*2*N+j] = Hesse(i,j);
        }
    }

    int_t nWSR = 800;

    QProblem mpc_qp_solver(2*N,1);
    mpc_qp_solver.init(H,g,A,lb,ub,lbA,ubA,nWSR);

    real_t x_solution[2*N];
    mpc_qp_solver.getPrimalSolution(x_solution);


    Vector2d u_k;
    MatrixXd U_result = MatrixXd::Zero(2,N); // 提取优化后的控制输入序列
    for(int i=0;i<N;i++)
    {
        u_k(0) = x_solution[2*i];
        u_k(1) = x_solution[2*i+1];
        U_result.col(i) = u_k;
//        std::cout<<"U "<<i+1<<" : "<<endl<<u_k<<endl;
    }
    //cout<<"N : "<<N<<endl;
    return U_result;
}

// MPC控制器初始化函数
// 从ROS参数服务器获取MPC参数
void MPC_controller::MPC_init(rclcpp::Node::SharedPtr nh)
{
    // 获取最大速度、最大角速度和权重参数
    nh->get_parameter("v_max", v_max);
    nh->get_parameter("w_max", w_max);
    nh->get_parameter("omega0", omega0);
    nh->get_parameter("omega1", omega1);

    // nh.param("/ego_planner_node/MPC/v_max",v_max,0.7);
    // nh.param("/ego_planner_node/MPC/w_max",w_max,0.3);
    RCLCPP_INFO(nh->get_logger(), "v_max : %f , w_max : %f", v_max, w_max);
    w_min = -w_max;
    v_min=0;

}
