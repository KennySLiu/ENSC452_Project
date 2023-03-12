`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/11/2023 09:14:41 PM
// Design Name: 
// Module Name: myDesign
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module kennys_TB();
    parameter DATA_WIDTH = 32;

    reg                           aclk;
    reg                           axi_resetn;
    //temp data interface
    reg   [DATA_WIDTH-1:0]        s_axis_temp_data; //[c,n,s,e,w]
    wire                          s_axis_temp_ready;
    reg                           s_axis_temp_valid;
      
    //result interface
    wire  [DATA_WIDTH-1:0]        m_axis_result_data;
    reg                           m_axis_result_ready;
    wire                          m_axis_result_valid;

    reg   [DATA_WIDTH-1:0]        temp_data[0:9];
    integer temp_counter;
    initial begin
        temp_data[0] = 32'h01c00000; //0.013671875
        temp_data[1] = 32'h02000000; //0.015625
        temp_data[2] = 32'h02400000;
        temp_data[3] = 32'h02800000;
        temp_data[4] = 32'h02c00000;
        temp_data[5] = 32'h06c00000;
        temp_data[6] = 32'h07c00000;
        temp_data[7] = 32'h04c00000;
        temp_data[8] = 32'h05400000;
        temp_data[9] = 32'h04400000;
        temp_counter = 0;     
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
            if(s_axis_temp_valid) begin
                s_axis_temp_data <= temp_data[temp_counter];
                if(temp_counter == 9) 
                    temp_counter = 0;
                else
                    temp_counter=temp_counter+1;
            end
        end
    end
    
    //control validity of temp data
    initial begin
        s_axis_temp_valid = 0;
        #24;
        s_axis_temp_valid = 1;
        s_axis_temp_data <= temp_data[0];
        #1;
        s_axis_temp_valid = 0;
//        #16;
//        s_axis_temp_valid = 0;
//        #8;
//        s_axis_temp_valid = 1;
//        #20;
//        s_axis_temp_valid = 0;
//        #8;
//        s_axis_temp_valid = 1;
//        #20;
//        s_axis_temp_valid = 0;
//        #8;
//        s_axis_temp_valid = 1;
//        #68;
//        s_axis_temp_valid = 0;
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
    
    
    //simulation time handle
    initial begin
        #400;
        $finish;
    end






    cordic_0 sqrt_cordic (
      .aclk(aclk),                                        // input wire aclk
      .s_axis_cartesian_tvalid(s_axis_temp_valid),  // input wire s_axis_cartesian_tvalid
      .s_axis_cartesian_tdata(s_axis_temp_data),    // input wire [31 : 0] s_axis_cartesian_tdata
      .m_axis_dout_tvalid(m_axis_result_valid),            // output wire m_axis_dout_tvalid
      .m_axis_dout_tdata(m_axis_result_data)              // output wire [31 : 0] m_axis_dout_tdata
    );
    
endmodule
