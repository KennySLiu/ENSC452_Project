`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Dilshan sent me this Testbench, which he used to test some modules.
// You can use this as a template.
// SOME OTHER NOTES: 
//	- To test a Xilinx IP, you need to make an RTL project (NOT make a block diagram).
// 	 	Then you can find the instantiation template in the Sources window > IP Sources.
//		It's a .veo file
// 
//////////////////////////////////////////////////////////////////////////////////


module Testbench();

    parameter DATA_WIDTH = 32;

    reg                           aclk;
    reg                           axi_resetn;
    //temp data interface
    reg   [DATA_WIDTH-1:0]        s_axis_temp_data; //[c,n,s,e,w]
    wire                          s_axis_temp_ready;
    reg                           s_axis_temp_valid;
    
    //power data interface
    reg   [DATA_WIDTH-1:0]        s_axis_power_data;
    wire                          s_axis_power_ready;
    reg                           s_axis_power_valid;
    
    //result interface
    wire  [DATA_WIDTH-1:0]        m_axis_result_data;
    reg                           m_axis_result_ready;
    wire                          m_axis_result_valid;
    //config data
    reg   [DATA_WIDTH-1:0]        cns;
    reg   [DATA_WIDTH-1:0]        cwe;
    reg   [DATA_WIDTH-1:0]        cc;
    reg   [DATA_WIDTH-1:0]        Cap_1;
    reg   [DATA_WIDTH-1:0]        c_amb;
    
    /*
    //testbench data store
    reg   [DATA_WIDTH*5-1:0]      temp_data[0:9];
    reg   [DATA_WIDTH-1:0]        power_data[0:9];
    integer temp_counter;
    integer power_counter;
    
    initial begin
        temp_data[0] = {32'h01c00000, 32'h02000000, 32'h02400000, 32'h02800000, 32'h02c00000}; //7, 8, 9, 10, 11
        temp_data[1] = {32'h06c00000, 32'h07c00000, 32'h04c00000, 32'h05400000, 32'h04400000}; //27, 31, 19, 21, 17
        temp_data[2] = {32'h03c00000, 32'h02c00000, 32'h01800000, 32'h01c00000, 32'h05000000}; //15	11	6	7	20
        temp_data[3] = {32'h00600000, 32'h01400000, 32'h0121b38d, 32'h05c80000, 32'h00c00000}; //1.5	5	4.526584	23.125	3
        temp_data[4] = {32'h00933333, 32'h02000000, 32'h009761f6, 32'h0096cfa2, 32'h005067ae}; //2.3	8	2.365354	2.356423	1.256328
        temp_data[5] = {32'h03eb851f, 32'h05e00000, 32'h05400000, 32'h00508ab1, 32'h039067f5}; //15.68	23.5	21	1.258465	14.256345
        temp_data[6] = {32'h05f1eb85, 32'h00974bc7, 32'h05800000, 32'h03906790, 32'h030f2431}; //23.78	2.364	22	14.256321	12.236584
        temp_data[7] = {32'h057289b1, 32'h01000000, 32'h030f138c, 32'h05d09308, 32'h0094d7ae}; //21.789654	4	12.235568	23.258974	2.325664
        temp_data[8] = {32'h05e5b48d, 32'h00976abe, 32'h025d5a3a, 32'h05dd53bd, 32'h029092b0}; //23.589145	2.36589	9.458632	23.458236	10.258953
        temp_data[9] = {32'h02871c71, 32'h05d6d8d8, 32'h02000000, 32'h008f21f3, 32'h024f18e3}; //10.111111	23.356985	8	2.236447	9.235894 
        
        power_data[0] = 32'h03000000;
        power_data[1] = 32'h09000000;
        power_data[2] = 32'h04cccccd;
        power_data[3] = 32'h02c00000;
        power_data[4] = 32'h030f11b6;
        power_data[5] = 32'h0096b4d0;
        power_data[6] = 32'h04cf1206;
        power_data[7] = 32'h03d7aa8b;
        power_data[8] = 32'h031700a8;
        power_data[9] = 32'h05ecb2c8;  
        
        temp_counter = 0;     
        power_counter = 0;
    end
    */
    
    //testbench data store
    reg   [DATA_WIDTH-1:0]        temp_data[0:9];
    reg   [DATA_WIDTH-1:0]        power_data[0:9];
    integer temp_counter;
    integer power_counter;
    
    initial begin
        temp_data[0] = 32'h01c00000; //7
        temp_data[1] = 32'h02000000; //8
        temp_data[2] = 32'h02400000; //9
        temp_data[3] = 32'h02800000; //10
        temp_data[4] = 32'h02c00000; //11
        temp_data[5] = 32'h06c00000; //27
        temp_data[6] = 32'h07c00000; //31
        temp_data[7] = 32'h04c00000; //19
        temp_data[8] = 32'h05400000; //21
        temp_data[9] = 32'h04400000; //17
        
        
        power_data[0] = 32'h03000000;
        power_data[1] = 32'h09000000;
        power_data[2] = 32'h04cccccd;
        power_data[3] = 32'h02c00000;
        power_data[4] = 32'h030f11b6;
        power_data[5] = 32'h0096b4d0;
        power_data[6] = 32'h04cf1206;
        power_data[7] = 32'h03d7aa8b;
        power_data[8] = 32'h031700a8;
        power_data[9] = 32'h05ecb2c8;  
        
        temp_counter = 0;     
        power_counter = 0;
    end
    
    //clock handle
    initial begin
        aclk = 0;
        forever #2 aclk = ~aclk;
    end
    
    //reset handle
    initial begin
        axi_resetn = 0;
        #7;
        axi_resetn = 1;
    end
    
    //passing temp data
    always @(negedge aclk) begin
        if(~axi_resetn) begin
            s_axis_temp_data <= {DATA_WIDTH{1'b0}};
        end
        else begin
            if(s_axis_temp_ready & s_axis_temp_valid) begin
                s_axis_temp_data <= temp_data[temp_counter];
                if(temp_counter == 9) 
                    temp_counter = 0;
                else
                    temp_counter=temp_counter+1;
            end
        end
    end
    
    
    //passing power data
    always @(negedge aclk) begin
        if(~axi_resetn) begin
            s_axis_power_data <= {DATA_WIDTH*{1'b0}};
        end
        else begin
            if(s_axis_power_ready & s_axis_power_valid) begin
                s_axis_power_data <= power_data[power_counter];
                if(power_counter == 9) 
                    power_counter = 0;
                else
                    power_counter=power_counter+1;
            end
        end
    end
    
    //control validity of temp data
    initial begin
        s_axis_temp_valid = 0;
        #24;
        s_axis_temp_valid = 1;
        s_axis_temp_data <= temp_data[0];
        #16;
        s_axis_temp_valid = 0;
        #8;
        s_axis_temp_valid = 1;
        #20;
        s_axis_temp_valid = 0;
        #8;
        s_axis_temp_valid = 1;
        #20;
        s_axis_temp_valid = 0;
        #8;
        s_axis_temp_valid = 1;
        #68;
        s_axis_temp_valid = 0;
    end
    
    //control validity of power data
    initial begin
        s_axis_power_valid = 0;
        #12;
        s_axis_power_valid = 1;
        s_axis_power_data <= power_data[0];
        #20;
        s_axis_power_valid = 0;
        #8;
        s_axis_power_valid = 1;
        #20;
        s_axis_power_valid = 0;
        #8;
        s_axis_power_valid = 1;
        #20;
        s_axis_power_valid = 0;
        #8;
        s_axis_power_valid = 1;
        #68;
        s_axis_power_valid = 0;
    end
    
    
    //handling master ready
    initial begin
        m_axis_result_ready = 0;
        #7;
        m_axis_result_ready = 1;
        #24;
        m_axis_result_ready = 0;
        #12;
        m_axis_result_ready = 1;
        #4;
        m_axis_result_ready = 0;
        #16;
        m_axis_result_ready = 1;
        #48;
        m_axis_result_ready = 0;
        #16;
        m_axis_result_ready = 1;
        
    end
    
    //config handle
    initial begin
        cns = 32'h00080000; //0.125
        cwe = 32'h0010624e; //0.256
        cc = 32'h0036c8b4; //0.856
        Cap_1 = 32'h00774bc7; //1.864
        c_amb = 32'h14bc28f6; //82.94
    end
   
    //simulation time handle
    initial begin
        #400;
        $finish;
    end
    
    topModule #(.DATA_WIDTH(32), .INT_WIDTH(10), .FLOAT_WIDTH(22)) topModule_inst(
                        .aclk(aclk),
                        .axi_resetn(axi_resetn),
                        //temp data interface
                        .s_axis_temp_data(s_axis_temp_data), 
                        .s_axis_temp_ready(s_axis_temp_ready),
                        .s_axis_temp_valid(s_axis_temp_valid),
                        //power data interface
                        .s_axis_power_data(s_axis_power_data),
                        .s_axis_power_ready(s_axis_power_ready),
                        .s_axis_power_valid(s_axis_power_valid),
                        //output temp interface
                        .m_axis_temp_data(m_axis_result_data), //[c,n,s,e,w]
                        .m_axis_temp_ready(m_axis_result_ready),
                        .m_axis_temp_valid(m_axis_result_valid),
                        //config data
                        .cns(cns),
                        .cwe(cwe),
                        .cc(cc),
                        .Cap_1(Cap_1),
                        .c_amb(c_amb)  
    );

endmodule