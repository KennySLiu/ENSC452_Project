`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 03/19/2023 06:55:55 PM
// Design Name: 
// Module Name: eq_streaming
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


module eq_streaming #(
	parameter integer C_S_AXI_DATA_WIDTH = 32
)
(
    input aclk,
    input aresetn,
    
    input [C_S_AXI_DATA_WIDTH-1:0] D_READY,
    input [C_S_AXI_DATA_WIDTH-1:0] FFT_PTNS,
    input [C_S_AXI_DATA_WIDTH-1:0] FFT_BKTS,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg0,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg1,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg2,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg3,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg4,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg5,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg6,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg7,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg8,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg9,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg10,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg11,
    input [C_S_AXI_DATA_WIDTH-1:0] my_reg12,
    
    input [C_S_AXI_DATA_WIDTH-1:0] s_axis_data,
    //input s_axis_last,
    output reg s_axis_ready = 1'b0,
    input s_axis_valid,
    
    output reg [C_S_AXI_DATA_WIDTH-1:0] m_axis_data,
    input m_axis_ready,
    //output reg m_axis_last = 1'b0,
    output reg m_axis_valid = 1'b0
);
    
    
    
//reg [7:0] state = 8'd0;
reg [7:0] j = 8'd1;
reg [31:0] my_bkts = 32'd1;
reg [31:0] bkt_c = 32'd1;
reg [31:0] eq_data = 32'd0;
reg [31:0] count = 32'd0;
reg [31:0] d_count = 32'd0;
reg signed [23:0] my_regA [12:0];
//reg l0 = 1'b0;
reg r0 = 1'b0;
reg v0 = 1'b0;
//reg l1 = 1'b0;
reg r1 = 1'b0;
reg v1 = 1'b0;
reg signed [47:0] re_result;
reg signed [47:0] img_result;

wire signed [23:0] real_P = {eq_data[15:0],8'd0};
wire signed [23:0] img_P = {eq_data[31:16],8'd0};

//localparam IDLE = 8'd0;
//localparam START_EQ = 8'd1;

always @(posedge aclk )
begin
    if ( aresetn == 1'b0 )
    begin
        my_regA[0] <= 0;
        my_regA[1] <= 0;
        my_regA[2] <= 0;
        my_regA[3] <= 0;
        my_regA[4] <= 0;
        my_regA[5] <= 0;
        my_regA[6] <= 0;
        my_regA[7] <= 0;
        my_regA[8] <= 0;
        my_regA[9] <= 0;
        my_regA[10] <= 0;
        my_regA[11] <= 0;
        my_regA[12] <= 0;
    end
    else begin
        my_regA[0] <= my_reg0[23:0];
        my_regA[1] <= my_reg1[23:0];
        my_regA[2] <= my_reg2[23:0];
        my_regA[3] <= my_reg3[23:0];
        my_regA[4] <= my_reg4[23:0];
        my_regA[5] <= my_reg5[23:0];
        my_regA[6] <= my_reg6[23:0];
        my_regA[7] <= my_reg7[23:0];
        my_regA[8] <= my_reg8[23:0];
        my_regA[9] <= my_reg9[23:0];
        my_regA[10] <= my_reg10[23:0];
        my_regA[11] <= my_reg11[23:0];
        my_regA[12] <= my_reg12[23:0];
    end
end


//always @(posedge aclk )
//begin
//    if ( aresetn == 1'b0 )
//    begin
//        //m_axis_last <= 1'b0;
//    end
//    else begin
//        //l0 <= s_axis_last;
//        //l1  <= l0;
//        //m_axis_last <= l1;
//    end
//end

always @(posedge aclk )
begin
    if ( aresetn == 1'b0 )
    begin
        m_axis_valid <= 1'b0;
    end
    else begin
        v0 <= s_axis_valid;
        v1  <= v0;
        m_axis_valid <= v1;
    end
end

always @(posedge aclk )
begin
    if ( aresetn == 1'b0 )
    begin
        s_axis_ready <= 1'b0;
    end
    else begin
        r0 <= m_axis_ready;
        r1  <= r0;
        s_axis_ready <= r1;
    end
end

always @(posedge aclk )
begin
    if ( aresetn == 1'b0 )
    begin
        d_count <= 1'b0;
    end
    else begin
        d_count <= count;
    end
end



//assign s_axis_ready = m_axis_ready;

always @(posedge aclk )
begin
    if ( aresetn == 1'b0 )
    begin
        count <= 32'd0;
        eq_data <= 32'd0;
        m_axis_data <= m_axis_data;
        j <= 8'd1;
        my_bkts <= 32'd1;
        bkt_c <= 32'd1;
    end
    else begin
//        if (count == 0)
//        begin 
        if(s_axis_valid & m_axis_ready) 
        begin
            if (count == (FFT_PTNS-1))
            begin 
                count <= 32'd0;
            end 
            else begin 
                count <= count + 1'b1;
            end
            
            eq_data <= s_axis_data;
        end 
//        end 
//        else begin 
//            if(s_axis_valid & s_axis_ready) 
//            begin
//                if (count == (FFT_PTNS-1))
//                begin 
//                    count <= 32'd0;
//                end 
//                else begin 
//                    count <= count + 1'b1;
//                end
                
//                eq_data <= s_axis_data;
//            end 
//        end
		
		if(v0 & r0)
		begin 
		    if(d_count == 0)
		    begin 
		      re_result <= (real_P*my_regA[0]);
              img_result <= (img_P*my_regA[0]);
//              m_axis_data[15:0] <= re_result[47:32];
//              m_axis_data[31:16] <= img_result[47:32];

		    end 
		    else begin
		      re_result <= (real_P*my_regA[j]);
		      img_result <= (img_P*my_regA[j]);
//		        m_axis_data[15:0] <= re_result[47:32];
//              m_axis_data[31:16] <= img_result[47:32];
              
//		        m_axis_data[14:0] <= ((eq_data[14:0] << 8)*my_regA[j])>>16;
//              m_axis_data[30:16] <= ((eq_data[30:16] << 8)*my_regA[j])>>16;
//              m_axis_data[15] <= eq_data[15];
//              m_axis_data[31] <= eq_data[31];
              if(my_bkts == bkt_c) 
                begin 
                    bkt_c <= 32'd1;
                    if (j == (FFT_BKTS))
                    begin 
                        my_bkts <= 32'd1;
                        j <= 8'd1;
                    end 
                    else begin 
                        j <= j + 8'd1;
                        my_bkts <= (my_bkts) << 1;
                    end 
                    
                end 
                else begin 
                    bkt_c <= bkt_c + 1'b1;
                end
		     end
		end
		
		if(v1 & r1)
		begin 
		    m_axis_data[15:0] <= re_result[31:16];
            m_axis_data[31:16] <= img_result[31:16];
		end	
	
    end
end    
    
endmodule

